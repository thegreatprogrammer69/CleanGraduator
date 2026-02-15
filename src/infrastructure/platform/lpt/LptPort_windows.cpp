#include "LptPort.h"
#include <windows.h>
#include <stdexcept>

namespace infra::platform {

using InitializeOlsFunc = BOOL (*)();
using DeinitializeOlsFunc = VOID (*)();
using ReadIoPortByteFunc = UCHAR (*)(USHORT);
using WriteIoPortByteFunc = VOID (*)(USHORT, UCHAR);

struct LptPort::LptPortImpl {
    HMODULE dll = nullptr;
    InitializeOlsFunc initializeOls = nullptr;
    DeinitializeOlsFunc deinitializeOls = nullptr;
    ReadIoPortByteFunc readIoPortByte = nullptr;
    WriteIoPortByteFunc writeIoPortByte = nullptr;
};

LptPort::LptPort() noexcept = default;

LptPort::~LptPort() {
    close();
}

void LptPort::open(unsigned short address) {
    if (opened_)
        throw std::runtime_error("LptPort already opened");

    impl_ = new LptPortImpl();

    impl_->dll = LoadLibraryW(L"WinRing0x64.dll");
    if (!impl_->dll)
        throw std::runtime_error("Failed to load WinRing0");

    impl_->initializeOls =
        reinterpret_cast<InitializeOlsFunc>(
            GetProcAddress(impl_->dll, "InitializeOls"));

    impl_->deinitializeOls =
        reinterpret_cast<DeinitializeOlsFunc>(
            GetProcAddress(impl_->dll, "DeinitializeOls"));

    impl_->readIoPortByte =
        reinterpret_cast<ReadIoPortByteFunc>(
            GetProcAddress(impl_->dll, "ReadIoPortByte"));

    impl_->writeIoPortByte =
        reinterpret_cast<WriteIoPortByteFunc>(
            GetProcAddress(impl_->dll, "WriteIoPortByte"));

    if (!impl_->initializeOls())
        throw std::runtime_error("InitializeOls failed");

    address_ = address;
    opened_ = true;
}

void LptPort::close() {
    if (!opened_)
        return;

    if (impl_) {
        if (impl_->deinitializeOls)
            impl_->deinitializeOls();

        if (impl_->dll)
            FreeLibrary(impl_->dll);

        delete impl_;
        impl_ = nullptr;
    }

    opened_ = false;
}

bool LptPort::isOpen() const noexcept {
    return opened_;
}

unsigned char LptPort::read(unsigned short offset) const {
    if (!opened_)
        throw std::runtime_error("LptPort not opened");

    return impl_->readIoPortByte(address_ + offset);
}

void LptPort::write(unsigned short offset, unsigned char value) {
    if (!opened_)
        throw std::runtime_error("LptPort not opened");

    impl_->writeIoPortByte(address_ + offset, value);
}

}
