#ifndef CLEANGRADUATOR_COMPORT_H
#define CLEANGRADUATOR_COMPORT_H
#include <string>

namespace infra::platform {

class ComPort final {
public:
    explicit ComPort(const std::string& port);
    ~ComPort();

    void flush();

    std::size_t read(unsigned char* buffer, std::size_t size);
    std::size_t write(const unsigned char* data, std::size_t size);

private:
    struct ComPortImpl* impl_;
};

}

#endif //CLEANGRADUATOR_COMPORT_H