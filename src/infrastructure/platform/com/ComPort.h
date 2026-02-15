#ifndef CLEANGRADUATOR_COMPORT_H
#define CLEANGRADUATOR_COMPORT_H

#include <string>
#include <cstddef>

namespace infra::platform {

    class ComPort final {
    public:
        ComPort() noexcept;
        ~ComPort();

        void open(const std::string& port);
        void close();

        bool isOpen() const noexcept;

        void flush();

        std::size_t read(unsigned char* buffer, std::size_t size);
        std::size_t write(const unsigned char* data, std::size_t size);

    private:
        struct ComPortImpl* impl_ = nullptr;
    };

}

#endif
