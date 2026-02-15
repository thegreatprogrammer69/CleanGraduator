#include "ComPort.h"
#include <windows.h>
#include <stdexcept>

namespace infra::platform {

struct ComPortImpl {
    HANDLE handle = INVALID_HANDLE_VALUE;
};

static bool is_open(const ComPortImpl* impl) {
    return impl && impl->handle != INVALID_HANDLE_VALUE;
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

    std::string full = "\\\\.\\" + port;

    HANDLE h = CreateFileA(
        full.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (h == INVALID_HANDLE_VALUE) {
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("CreateFile failed");
    }

    impl_->handle = h;

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(h, &dcb))
        throw std::runtime_error("GetCommState failed");

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;

    if (!SetCommState(h, &dcb))
        throw std::runtime_error("SetCommState failed");

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout         = 1;
    timeouts.ReadTotalTimeoutConstant    = 500;
    timeouts.ReadTotalTimeoutMultiplier  = 8;
    timeouts.WriteTotalTimeoutConstant   = 500;
    timeouts.WriteTotalTimeoutMultiplier = 8;

    SetCommTimeouts(h, &timeouts);
}

void ComPort::close() {
    if (!is_open(impl_))
        return;

    CloseHandle(impl_->handle);
    impl_->handle = INVALID_HANDLE_VALUE;

    delete impl_;
    impl_ = nullptr;
}

bool ComPort::isOpen() const noexcept {
    return is_open(impl_);
}

void ComPort::flush() {
    if (!isOpen()) return;
    PurgeComm(impl_->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

std::size_t ComPort::read(unsigned char* buffer, std::size_t size) {
    if (!isOpen() || !buffer || size == 0) return 0;

    DWORD bytesRead = 0;
    if (!ReadFile(impl_->handle, buffer, (DWORD)size, &bytesRead, nullptr))
        return 0;

    return bytesRead;
}

std::size_t ComPort::write(const unsigned char* data, std::size_t size) {
    if (!isOpen() || !data || size == 0) return 0;

    DWORD bytesWritten = 0;
    if (!WriteFile(impl_->handle, data, (DWORD)size, &bytesWritten, nullptr))
        return 0;

    return bytesWritten;
}

}
