#include "ComPort.h"
#include <windows.h>
#include <stdexcept>
#include <string>

namespace {

std::wstring to_wstring(const std::string& str) {
    if (str.empty()) return {};

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size_needed - 1, 0);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr.data(), size_needed);
    return wstr;
}

std::string get_last_error_message(DWORD error) {
    if (error == 0) return {};

    LPSTR buffer = nullptr;

    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer,
        0,
        nullptr
    );

    std::string message(buffer, size);
    LocalFree(buffer);

    return message;
}

[[noreturn]] void throw_last_error(const std::string& prefix) {
    DWORD err = GetLastError();
    std::string msg = prefix +
                      " (code=" + std::to_string(err) +
                      "): " + get_last_error_message(err);
    throw std::runtime_error(msg);
}

} // anonymous namespace

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
    std::wstring wfull = to_wstring(full);

    HANDLE h = CreateFileW(
        wfull.c_str(),
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
        throw_last_error("CreateFileW failed");
    }

    impl_->handle = h;

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(h, &dcb))
        throw_last_error("GetCommState failed");

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;

    if (!SetCommState(h, &dcb))
        throw_last_error("SetCommState failed");

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout         = 1;
    timeouts.ReadTotalTimeoutConstant    = 500;
    timeouts.ReadTotalTimeoutMultiplier  = 8;
    timeouts.WriteTotalTimeoutConstant   = 500;
    timeouts.WriteTotalTimeoutMultiplier = 8;

    if (!SetCommTimeouts(h, &timeouts))
        throw_last_error("SetCommTimeouts failed");

    PurgeComm(impl_->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
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

    if (!PurgeComm(impl_->handle, PURGE_RXCLEAR | PURGE_TXCLEAR))
        throw_last_error("PurgeComm failed");
}

std::size_t ComPort::read(unsigned char* buffer, std::size_t size) {
    if (!isOpen() || !buffer || size == 0) return 0;

    DWORD bytesRead = 0;
    if (!ReadFile(impl_->handle, buffer, (DWORD)size, &bytesRead, nullptr))
        throw_last_error("ReadFile failed");

    return bytesRead;
}

std::size_t ComPort::write(const unsigned char* data, std::size_t size) {
    if (!isOpen() || !data || size == 0) return 0;

    DWORD bytesWritten = 0;
    if (!WriteFile(impl_->handle, data, (DWORD)size, &bytesWritten, nullptr))
        throw_last_error("WriteFile failed");

    return bytesWritten;
}

} // namespace infra::platform