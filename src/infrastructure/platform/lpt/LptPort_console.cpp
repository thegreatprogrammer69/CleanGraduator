#include "LptPort.h"

#include <iostream>
#include <array>
#include <stdexcept>

namespace infra::platform {

    struct LptPort::LptPortImpl {
        std::array<unsigned char, 8> registers{}; // 8 регистров достаточно для LPT
    };

    LptPort::LptPort() noexcept = default;

    LptPort::~LptPort() {
        close();
    }

    void LptPort::open(unsigned short address) {
        if (opened_) {
            std::cout << "[LPT] Already opened\n";
            return;
        }

        address_ = address;
        impl_ = new LptPortImpl{};
        opened_ = true;

        std::cout << "[LPT] Open console port at 0x"
                  << std::hex << address_ << std::dec << "\n";
    }

    void LptPort::close() noexcept {
        if (!opened_)
            return;

        std::cout << "[LPT] Close port 0x"
                  << std::hex << address_ << std::dec << "\n";

        delete impl_;
        impl_ = nullptr;
        opened_ = false;
    }

    bool LptPort::isOpen() const noexcept {
        return opened_;
    }

    unsigned char LptPort::read(unsigned short offset) const {
        if (!opened_ || !impl_)
            throw std::runtime_error("LptPort not opened");

        if (offset >= impl_->registers.size())
            throw std::out_of_range("LptPort offset out of range");

        auto value = impl_->registers[offset];

        std::cout << "[LPT] READ  reg[" << offset << "] = "
                  << static_cast<int>(value) << "\n";

        return value;
    }

    unsigned char LptPort::read() const {
        return read(0);
    }

    void LptPort::write(unsigned short offset, unsigned char value) {
        if (!opened_ || !impl_)
            throw std::runtime_error("LptPort not opened");

        if (offset >= impl_->registers.size())
            throw std::out_of_range("LptPort offset out of range");

        impl_->registers[offset] = value;

        std::cout << "[LPT] WRITE reg[" << offset << "] = "
                  << static_cast<int>(value) << "\n";
    }

    void LptPort::write(unsigned char value) {
        write(0, value);
    }

} // namespace infra::platform
