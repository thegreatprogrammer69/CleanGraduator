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
#include "domain/ports/outbound/IClock.h"
#include "domain/ports/outbound/ILogger.h"


namespace infra::camera {

static int xioctl(int fd, unsigned long request, void* arg) {
    for (;;) {
        int r = ::ioctl(fd, request, arg);
        if (r == -1 && errno == EINTR) continue;
        return r;
    }
}

V4LCamera::V4LCamera(const CameraPorts& ports, CameraConfig config)
    : logger_(ports.logger), ports_(ports), config_(std::move(config)) {}

V4LCamera::~V4LCamera() {
    stop();
}

void V4LCamera::addSink(domain::ports::IVideoSink& sink) {
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    sinks_.push_back(&sink);
}

void V4LCamera::removeSink(domain::ports::IVideoSink& sink) {
    std::lock_guard<std::mutex> lock(sinks_mutex_);
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), &sink), sinks_.end());
}

bool V4LCamera::start() {
    if (running_.load()) return true;

    fd_ = ::open(config_.source.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (fd_ < 0) {
        logger_.error("logger_open failed: {}", std::strerror(errno));
        return false;
    }

    // Проверим capability
    v4l2_capability cap{};
    if (xioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) {
        logger_.error("logger_VIDIOC_QUERYCAP failed: {}", std::strerror(errno));
        stop();
        return false;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
        !(cap.capabilities & V4L2_CAP_STREAMING)) {
        logger_.error("logger_device does not support capture/streaming");
        stop();
        return false;
    }

    // Формат: YUYV 640x480
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = config_.width;
    fmt.fmt.pix.height = config_.height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (xioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
        logger_.error("logger_VIDIOC_S_FMT failed: {}", std::strerror(errno));
        stop();
        return false;
    }

    // FPS: 30
    v4l2_streamparm parm{};
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = config_.fps;

    if (xioctl(fd_, VIDIOC_S_PARM, &parm) < 0) {
        logger_.warn("logger_VIDIOC_S_PARM failed (device may ignore): {}", std::strerror(errno));
        // не фейлим — некоторые драйверы игнорируют
    }

    // Запрос буферов
    v4l2_requestbuffers req{};
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
        logger_.error("logger_VIDIOC_REQBUFS failed: {}", std::strerror(errno));
        stop();
        return false;
    }
    if (req.count < 2) {
        logger_.error("logger_insufficient buffer memory");
        stop();
        return false;
    }

    buffers_.resize(req.count);

    for (uint32_t i = 0; i < req.count; ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) {
            logger_.error("logger_VIDIOC_QUERYBUF failed: {}", std::strerror(errno));
            stop();
            return false;
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
            logger_.error("logger_mmap failed: {}", std::strerror(errno));
            stop();
            return false;
        }
    }

    // Очередим буферы
    for (uint32_t i = 0; i < buffers_.size(); ++i) {
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
            logger_.error("logger_VIDIOC_QBUF failed: {}", std::strerror(errno));
            stop();
            return false;
        }
    }

    // Старт стриминга
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
        logger_.error("logger_VIDIOC_STREAMON failed: {}", std::strerror(errno));
        stop();
        return false;
    }

    running_.store(true);
    thread_ = std::thread(&V4LCamera::captureLoop, this);
    logger_.info("Camera {} successfully opened", config_.source);
    return true;
}

void V4LCamera::stop() {
    bool wasRunning = running_.exchange(false);

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

    (void)wasRunning;
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
}

void V4LCamera::dispatchFrame(const uint8_t* data, size_t size) {
    auto frame = std::make_shared<domain::common::VideoFrame>();
    frame->width  = config_.width;
    frame->height = config_.height;
    frame->format = domain::common::PixelFormat::YUYV;   // убедись что у тебя есть такой формат
    frame->buffer = domain::common::VideoBuffer(static_cast<int>(size));

    std::memcpy(frame->buffer.data, data, size);

    const domain::common::Timestamp ts = ports_.clock.now();

    std::vector<domain::ports::IVideoSink*> sinks_copy;
    {
        std::lock_guard<std::mutex> lock(sinks_mutex_);
        sinks_copy = sinks_;
    }

    // logger_.info("Frame captured: {}", *frame);

    for (auto* sink : sinks_copy) {
        sink->onVideoFrame(ts, frame);
    }
}

}
