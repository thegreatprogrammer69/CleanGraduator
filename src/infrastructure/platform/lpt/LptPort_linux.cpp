#include "LptPort.h"

#include <stdexcept>
#include <sys/io.h>
#include <unistd.h>

namespace infra::platform {

    struct LptPort::LptPortImpl {
        // ничего не нужно
    };

    LptPort::LptPort() noexcept = default;

    LptPort::~LptPort() {
        close();
    }

    void LptPort::open(unsigned short address) {
        if (opened_)
            throw std::runtime_error("LptPort already opened");

        if (geteuid() != 0)
            throw std::runtime_error("LptPort requires root privileges");

        if (ioperm(address, 3, 1) != 0)
            throw std::runtime_error("ioperm failed");

        address_ = address;
        opened_ = true;
    }

    void LptPort::close() noexcept {
        if (!opened_)
            return;

        ioperm(address_, 3, 0);
        opened_ = false;
    }

    bool LptPort::isOpen() const noexcept {
        return opened_;
    }

    unsigned char LptPort::read(unsigned short offset) const {
        return inb(address_ + offset);
    }

    unsigned char LptPort::read() const {
        return inb(address_);
    }

    void LptPort::write(unsigned short offset, unsigned char value) {
        outb(value, address_ + offset);
    }

    void LptPort::write(unsigned char value) {
        outb(value, address_);
    }
}
