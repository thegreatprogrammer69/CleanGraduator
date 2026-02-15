#include "ComPort.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace infra::platform {

struct ComPortImpl {
    int fd = -1;
};

static bool is_open(const ComPortImpl* impl) {
    return impl && impl->fd >= 0;
}

ComPort::ComPort() noexcept = default;

ComPort::~ComPort() {
    close();
}

void ComPort::open(const std::string& port) {
    if (isOpen())
        throw std::runtime_error("ComPort already opened");

    if (port.empty())
        throw std::invalid_argument("ComPort: empty port name");

    impl_ = new ComPortImpl();

    std::string path =
        (port.rfind("/dev/", 0) == 0) ? port : "/dev/" + port;

    int fd = ::open(path.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error(
            "ComPort open failed: " + std::string(std::strerror(errno))
        );
    }

    impl_->fd = fd;

    termios tty{};
    if (tcgetattr(fd, &tty) != 0)
        throw std::runtime_error("tcgetattr failed");

    cfmakeraw(&tty);

    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag |= (CLOCAL | CREAD);

#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        throw std::runtime_error("tcsetattr failed");

    tcflush(fd, TCIOFLUSH);
}

void ComPort::close() {
    if (!is_open(impl_))
        return;

    ::close(impl_->fd);
    impl_->fd = -1;

    delete impl_;
    impl_ = nullptr;
}

bool ComPort::isOpen() const noexcept {
    return is_open(impl_);
}

void ComPort::flush() {
    if (!isOpen()) return;
    tcflush(impl_->fd, TCIOFLUSH);
}

std::size_t ComPort::read(unsigned char* buffer, std::size_t size) {
    if (!isOpen() || !buffer || size == 0) return 0;

    ssize_t n = ::read(impl_->fd, buffer, size);
    if (n <= 0) return 0;

    return static_cast<std::size_t>(n);
}

std::size_t ComPort::write(const unsigned char* data, std::size_t size) {
    if (!isOpen() || !data || size == 0) return 0;

    ssize_t n = ::write(impl_->fd, data, size);
    if (n <= 0) return 0;

    return static_cast<std::size_t>(n);
}

}
