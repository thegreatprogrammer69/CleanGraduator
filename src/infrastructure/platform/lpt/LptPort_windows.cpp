#include "LptPort.h"
#include <windows.h>
#include <stdexcept>

namespace infra::platform {

// Типы функций WinRing0
using InitializeOlsFunc = BOOL (*)();
using DeinitializeOlsFunc = VOID (*)();
using ReadIoPortByteFunc = UCHAR (*)(USHORT);
using WriteIoPortByteFunc = VOID (*)(USHORT, UCHAR);

class LptPortImpl {
public:
    HMODULE dll = nullptr;
    InitializeOlsFunc initializeOls = nullptr;
    DeinitializeOlsFunc deinitializeOls = nullptr;
    ReadIoPortByteFunc readIoPortByte = nullptr;
    WriteIoPortByteFunc writeIoPortByte = nullptr;
};

LptPort::LptPort(unsigned short address) {
    impl_ = new LptPortImpl();
    address_ = address;

    impl_->dll = LoadLibraryW(L"WinRing0x64.dll");
    if (!impl_->dll) {
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("Failed to load WinRing0 DLL");
    }

    impl_->initializeOls =
        reinterpret_cast<InitializeOlsFunc>(GetProcAddress(impl_->dll, "InitializeOls"));
    impl_->deinitializeOls =
        reinterpret_cast<DeinitializeOlsFunc>(GetProcAddress(impl_->dll, "DeinitializeOls"));
    impl_->readIoPortByte =
        reinterpret_cast<ReadIoPortByteFunc>(GetProcAddress(impl_->dll, "ReadIoPortByte"));
    impl_->writeIoPortByte =
        reinterpret_cast<WriteIoPortByteFunc>(GetProcAddress(impl_->dll, "WriteIoPortByte"));

    if (!impl_->initializeOls ||
        !impl_->deinitializeOls ||
        !impl_->readIoPortByte ||
        !impl_->writeIoPortByte) {

        FreeLibrary(impl_->dll);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("Failed to get WinRing0 functions");
    }

    if (!impl_->initializeOls()) {
        FreeLibrary(impl_->dll);
        delete impl_;
        impl_ = nullptr;
        throw std::runtime_error("InitializeOls failed (check driver installation and admin rights)");
    }
}

LptPort::~LptPort() {
    if (impl_) {
        if (impl_->deinitializeOls) {
            impl_->deinitializeOls();
        }
        if (impl_->dll) {
            FreeLibrary(impl_->dll);
        }
        delete impl_;
        impl_ = nullptr;
    }
}

unsigned char LptPort::read(unsigned short offset) const {
    if (!impl_ || !impl_->readIoPortByte) return 0;
    return impl_->readIoPortByte(address_ + offset);
}

    //    void write(unsigned short offset, unsigned char value);
void LptPort::write(unsigned short offset, unsigned char value) {
    if (!impl_ || !impl_->writeIoPortByte) return;
    impl_->writeIoPortByte(address_ + offset, value);
}

}