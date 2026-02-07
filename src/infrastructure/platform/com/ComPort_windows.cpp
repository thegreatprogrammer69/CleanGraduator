#include "ComPort.h"

#include <windows.h>

#include <cstdio>
#include <stdexcept>
#include <string>

namespace infra::platform {

static std::wstring string_to_wstring_utf8(const std::string& str)
{
    if (str.empty()) return {};

    int size_needed = MultiByteToWideChar(
        CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0
    );
    if (size_needed <= 0) return {};

    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(
        CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed
    );
    return wstr;
}

struct ComPortImpl {
    HANDLE handle = INVALID_HANDLE_VALUE;
};

static bool is_open(const ComPortImpl* impl)
{
    return impl && impl->handle != INVALID_HANDLE_VALUE;
}

ComPort::ComPort(const std::string& port)
    : impl_(new ComPortImpl)
{
    if (port.empty()) {
        delete impl_;
        impl_ = nullptr;
        throw std::invalid_argument("ComPort: empty port name");
    }

    // Точно как в примере: "\\\\.\\%s"
    char fullName[64];
    std::snprintf(fullName, sizeof(fullName), "\\\\.\\%s", port.c_str());

    impl_->handle = CreateFileW(
        string_to_wstring_utf8(fullName).c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (impl_->handle == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("ComPort: CreateFileW failed, winapi error=" + std::to_string(err));
    }

    // === DCB === (9600 8N1)
    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(impl_->handle, &dcb)) {
        DWORD err = GetLastError();
        CloseHandle(impl_->handle);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("ComPort: GetCommState failed, winapi error=" + std::to_string(err));
    }

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;

    if (!SetCommState(impl_->handle, &dcb)) {
        DWORD err = GetLastError();
        CloseHandle(impl_->handle);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("ComPort: SetCommState failed, winapi error=" + std::to_string(err));
    }

    // === TIMEOUTS === (как в примере)
    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout         = 1;
    timeouts.ReadTotalTimeoutConstant    = 500;
    timeouts.ReadTotalTimeoutMultiplier  = 8;
    timeouts.WriteTotalTimeoutConstant   = 500;
    timeouts.WriteTotalTimeoutMultiplier = 8;

    // В примере ошибка SetCommTimeouts не фейлит open, поэтому так же:
    (void)SetCommTimeouts(impl_->handle, &timeouts);
}

ComPort::~ComPort()
{
    if (is_open(impl_)) {
        CloseHandle(impl_->handle);
        impl_->handle = INVALID_HANDLE_VALUE;
    }
    delete impl_;
    impl_ = nullptr;
}

void ComPort::flush()
{
    if (!is_open(impl_)) return;

    // Точно как в примере
    (void)PurgeComm(impl_->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

std::size_t ComPort::read(unsigned char* buffer, std::size_t size)
{
    if (!is_open(impl_) || buffer == nullptr || size == 0) return 0;

    DWORD bytesRead = 0;
    if (!ReadFile(
            impl_->handle,
            buffer,
            static_cast<DWORD>(size),
            &bytesRead,
            nullptr
        )) {
        return 0;
    }

    return static_cast<std::size_t>(bytesRead);
}

std::size_t ComPort::write(const unsigned char* data, std::size_t size)
{
    if (!is_open(impl_) || data == nullptr || size == 0) return 0;

    DWORD bytesWritten = 0;
    if (!WriteFile(
            impl_->handle,
            data,
            static_cast<DWORD>(size),
            &bytesWritten,
            nullptr
        )) {
        return 0;
        }

    return static_cast<std::size_t>(bytesWritten);
}

}