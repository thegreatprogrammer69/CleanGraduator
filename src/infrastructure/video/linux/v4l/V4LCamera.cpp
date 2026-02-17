#include "V4LCamera.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>

#include <linux/videodev2.h>

#include "domain/core/measurement/Timestamp.h"
#include "domain/core/video/VideoFrame.h"
#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/video/VideoSourceError.h"
#include "domain/ports/clock/IClock.h"
#include "domain/ports/logging/ILogger.h"




namespace infra::camera {

static int xioctl(int fd, unsigned long request, void* arg) {
    for (;;) {
        int r = ::ioctl(fd, request, arg);
        if (r == -1 && errno == EINTR) continue;
        return r;
    }
}

V4LCamera::V4LCamera(VideoSourcePorts ports, V4LCameraConfig config)
    : logger_(ports.logger), ports_(ports), config_(std::move(config)) {}

V4LCamera::~V4LCamera() {
    close();
}

bool V4LCamera::open() {

    if (running_.load()) return true;

    auto abort_opening = [this]() {
        this->close();
        const domain::common::VideoSourceError err{logger_.lastError()};
        notifier_.notifyOpenFailed(err);
    };

    fd_ = ::open(config_.source.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (fd_ < 0) {
        logger_.error("V4LCamera::open() error: open failed: {}", std::strerror(errno));
        abort_opening(); return false;
    }

    // Проверим capability
    v4l2_capability cap{};
    if (xioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) {
        logger_.error("V4LCamera::open() error: VIDIOC_QUERYCAP failed: {}", std::strerror(errno));
        abort_opening(); return false;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
        !(cap.capabilities & V4L2_CAP_STREAMING)) {
        logger_.error("V4LCamera::open() error: device does not support capture/streaming");
        abort_opening(); return false;
    }

    // Формат: YUYV 640x480
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = config_.width;
    fmt.fmt.pix.height = config_.height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (xioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
        logger_.error("V4LCamera::open() error: VIDIOC_S_FMT failed: {}", std::strerror(errno));
        abort_opening(); return false;
    }

    // FPS: 30
    v4l2_streamparm parm{};
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = config_.fps;

    if (xioctl(fd_, VIDIOC_S_PARM, &parm) < 0) {
        logger_.warn("V4LCamera::open() error: VIDIOC_S_PARM failed (device may ignore): {}", std::strerror(errno));
        // не фейлим — некоторые драйверы игнорируют
    }

    // Запрос буферов
    v4l2_requestbuffers req{};
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
        logger_.error("V4LCamera::open() error: VIDIOC_REQBUFS failed: {}", std::strerror(errno));
        abort_opening(); return false;
    }
    if (req.count < 2) {
        logger_.error("V4LCamera::open() error: insufficient buffer memory");
        abort_opening(); return false;
    }

    buffers_.resize(req.count);

    for (uint32_t i = 0; i < req.count; ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) {
            logger_.error("V4LCamera::open() error: VIDIOC_QUERYBUF failed: {}", std::strerror(errno));
            abort_opening(); return false;
        }

        buffers_[i].length = buf.length;
        buffers_[i].start = ::mmap(
            nullptr,
            buf.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd_,
            buf.m.offset
        );

        if (buffers_[i].start == MAP_FAILED) {
            logger_.error("V4LCamera::open() error: mmap failed: {}", std::strerror(errno));
            abort_opening(); return false;
        }
    }

    // Очередим буферы
    for (uint32_t i = 0; i < buffers_.size(); ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
            logger_.error("V4LCamera::open() error: VIDIOC_QBUF failed: {}", std::strerror(errno));
            abort_opening(); return false;
        }
    }

    // Старт стриминга
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
        logger_.error("V4LCamera::open() error: VIDIOC_STREAMON failed: {}", std::strerror(errno));
        abort_opening(); return false;
    }

    running_.store(true);
    thread_ = std::thread(&V4LCamera::captureLoop, this);
    logger_.info("camera {} successfully opened", config_.source);

    notifier_.notifyOpened();

    return true;
}

void V4LCamera::close() {
    if (!running_.exchange(false)) return;

    logger_.info("closing camera {}", config_.source);

    if (thread_.joinable()) {
        thread_.join();
    }

    if (fd_ >= 0) {
        // STREAMOFF (даже если уже остановлено — ок)
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd_, VIDIOC_STREAMOFF, &type);

        for (auto& b : buffers_) {
            if (b.start && b.start != MAP_FAILED) {
                ::munmap(b.start, b.length);
                b.start = nullptr;
                b.length = 0;
            }
        }
        buffers_.clear();

        ::close(fd_);
        fd_ = -1;
    }

    logger_.info("camera {} successfully closed", config_.source);
}

void V4LCamera::addObserver(domain::ports::IVideoSourceObserver &o) {
    notifier_.addObserver(o);
}

void V4LCamera::removeObserver(domain::ports::IVideoSourceObserver &o) {
    notifier_.removeObserver(o);
}

void V4LCamera::captureLoop() {
    while (running_.load()) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd_, &fds);

        // Таймаут чтобы корректно реагировать на stop()
        timeval tv{};
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int r = ::select(fd_ + 1, &fds, nullptr, nullptr, &tv);
        if (r == -1) {
            if (errno == EINTR) continue;
            logger_.error("logger_select failed: {}", std::strerror(errno));
            break;
        }
        if (r == 0) {
            continue; // timeout
        }

        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (xioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) continue;
            logger_.error("logger_VIDIOC_DQBUF failed: {}", std::strerror(errno));
            break;
        }

        if (buf.index < buffers_.size() && buf.bytesused > 0) {
            auto* data = static_cast<uint8_t*>(buffers_[buf.index].start);
            dispatchFrame(data, buf.bytesused);
        }

        if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
            logger_.error("logger_VIDIOC_QBUF failed: {}", std::strerror(errno));
            break;
        }
    }
    notifier_.notifyClosed({});
}

void V4LCamera::dispatchFrame(const uint8_t* data, size_t size) {
    auto frame = std::make_shared<domain::common::VideoFrame>();
    frame->width  = config_.width;
    frame->height = config_.height;
    frame->format = domain::common::PixelFormat::YUYV;   // убедись что у тебя есть такой формат
    frame->buffer = domain::common::VideoBuffer(static_cast<int>(size));

    std::memcpy(frame->buffer.data, data, size);

    const domain::common::Timestamp ts = ports_.clock.now();

    domain::common::VideoFramePacket packet;
    packet.timestamp = ts;
    packet.frame = frame;

    notifier_.notifyFrame(packet);
}

}
