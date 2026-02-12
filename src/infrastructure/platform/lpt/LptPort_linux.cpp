#include "LptPort.h"

#include <stdexcept>
#include <sys/io.h>
#include <unistd.h>

namespace infra::platform {

LptPort::LptPort(unsigned short address) {
    impl_ = nullptr;
    address_ = address;

    // Проверка root
    if (geteuid() != 0) {
        throw std::runtime_error("LptPort requires root privileges");
    }

    if (ioperm(address_, 3, 1) != 0) {
        throw std::runtime_error("ioperm failed (check kernel setup and privileges)");
    }
}

LptPort::~LptPort() {
    // Забираем разрешения обратно
    ioperm(address_, 3, 0);
}

unsigned char LptPort::read(unsigned short offset) const {
    return inb(address_ + offset);
}

void LptPort::write(unsigned short offset, unsigned char value) {
    outb(value, address_ + offset);
}

}