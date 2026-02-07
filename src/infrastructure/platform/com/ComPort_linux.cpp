#include "ComPort.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace infra::platform {

struct ComPortImpl {
    int fd = -1;
};

static bool is_open(const ComPortImpl* impl) {
    return impl && impl->fd >= 0;
}

static speed_t to_speed_9600() {
    return B9600;
}

ComPort::ComPort(const std::string& port)
    : impl_(new ComPortImpl) {
    if (port.empty()) {
        delete impl_;
        impl_ = nullptr;
        throw std::invalid_argument("ComPort: empty port name");
    }

    // Если пользователь передал "ttyUSB0" / "ttyS0" — добавим /dev/
    // Если передал полный путь "/dev/ttyUSB0" — используем как есть.
    std::string path;
    if (port.rfind("/dev/", 0) == 0) {
        path = port;
    } else {
        path = "/dev/" + port;
    }

    // Открываем как обычный последовательный порт (без controlling terminal),
    // чтение/запись, синхронно.
    // Неблокирующий режим НЕ включаем, чтобы поведение было ближе к ReadFile/таймаутам;
    // таймауты зададим через termios (VMIN/VTIME).
    int fd = ::open(path.c_str(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        int e = errno;
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error(
            "ComPort: open failed for " + path + ", errno=" + std::to_string(e) +
            " (" + std::string(std::strerror(e)) + ")"
        );
    }
    impl_->fd = fd;

    // Заберём текущие настройки
    termios tty{};
    if (tcgetattr(impl_->fd, &tty) != 0) {
        int e = errno;
        ::close(impl_->fd);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error(
            "ComPort: tcgetattr failed, errno=" + std::to_string(e) +
            " (" + std::string(std::strerror(e)) + ")"
        );
    }

    // === Эквивалент 9600 8N1 ===
    // Сырой режим, без обработки спецсимволов/эха, без XON/XOFF и т.п.
    cfmakeraw(&tty);

    // 8 бит
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    // No parity
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~PARODD;

    // 1 stop bit
    tty.c_cflag &= ~CSTOPB;

    // Включим приемник, локальный режим
    tty.c_cflag |= (CLOCAL | CREAD);

    // Отключим аппаратный flow control (RTS/CTS)
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    // Отключим программный flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Baudrate 9600
    speed_t spd = to_speed_9600();
    if (cfsetispeed(&tty, spd) != 0 || cfsetospeed(&tty, spd) != 0) {
        int e = errno;
        ::close(impl_->fd);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error(
            "ComPort: cfsetispeed/cfsetospeed failed, errno=" + std::to_string(e) +
            " (" + std::string(std::strerror(e)) + ")"
        );
    }

    // === Таймауты, приближенно к Windows-варианту ===
    //
    // В Windows:
    // ReadIntervalTimeout = 1
    // ReadTotalTimeoutConstant = 500
    // ReadTotalTimeoutMultiplier = 8
    //
    // В POSIX termios есть VMIN/VTIME:
    // - VTIME в десятых долях секунды (deciseconds)
    // - работает как "интер-байтовый" таймаут в canonical off/raw режимах.
    //
    // Ближайшая практичная настройка:
    // - VMIN = 0 (разрешить вернуть 0 байт по таймауту)
    // - VTIME = 5 (0.5 секунды) ~ ReadTotalTimeoutConstant 500ms
    //
    // Мультипликатор по байтам напрямую не реализуется termios-ом;
    // если нужно один-в-один, это делается через poll/select + ручной расчёт.
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5; // 0.5s

    if (tcsetattr(impl_->fd, TCSANOW, &tty) != 0) {
        int e = errno;
        ::close(impl_->fd);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error(
            "ComPort: tcsetattr failed, errno=" + std::to_string(e) +
            " (" + std::string(std::strerror(e)) + ")"
        );
    }

    // Очистим очереди ввода/вывода как стартовую "промывку"
    (void)tcflush(impl_->fd, TCIOFLUSH);
}

ComPort::~ComPort()
{
    if (is_open(impl_)) {
        ::close(impl_->fd);
        impl_->fd = -1;
    }
    delete impl_;
    impl_ = nullptr;
}

void ComPort::flush()
{
    if (!is_open(impl_)) return;
    (void)tcflush(impl_->fd, TCIOFLUSH); // аналог PurgeComm RX/TX clear
}

std::size_t ComPort::read(unsigned char* buffer, std::size_t size)
{
    if (!is_open(impl_) || buffer == nullptr || size == 0) return 0;

    ssize_t n = ::read(impl_->fd, buffer, size);
    if (n < 0) {
        // При VMIN=0/VTIME>0 read обычно не возвращает -1 по таймауту,
        // но на всякий случай обработаем прерывания.
        if (errno == EINTR) return 0;
        return 0;
    }
    return static_cast<std::size_t>(n);
}

std::size_t ComPort::write(const unsigned char* data, std::size_t size)
{
    if (!is_open(impl_) || data == nullptr || size == 0) return 0;

    // Пишем "как есть". Можно сделать цикл для гарантированной записи всего буфера.
    ssize_t n = ::write(impl_->fd, data, size);
    if (n < 0) {
        if (errno == EINTR) return 0;
        return 0;
    }
    return static_cast<std::size_t>(n);
}

}