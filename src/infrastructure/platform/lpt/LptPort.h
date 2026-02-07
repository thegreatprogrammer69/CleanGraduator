#ifndef CLEANGRADUATOR_LPTPORT_H
#define CLEANGRADUATOR_LPTPORT_H

namespace infra::platform {

class LptPort {
public:
    LptPort(unsigned short address);
    ~LptPort();
    unsigned char read(unsigned short offset) const;
    void write(unsigned short offset, unsigned char value);

private:
    struct LptPortImpl *impl_;
    unsigned short address_;
};

}
#endif //CLEANGRADUATOR_LPTPORT_H