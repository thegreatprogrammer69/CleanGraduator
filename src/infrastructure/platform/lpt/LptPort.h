#ifndef CLEANGRADUATOR_LPTPORT_H
#define CLEANGRADUATOR_LPTPORT_H

namespace infra::platform {

    class LptPort {
    public:
        LptPort() noexcept;
        ~LptPort();

        void open(unsigned short address);
        void close();

        bool isOpen() const noexcept;

        unsigned char read(unsigned short offset) const;
        void write(unsigned short offset, unsigned char value);

    private:
        struct LptPortImpl;
        LptPortImpl* impl_ = nullptr;
        unsigned short address_ = 0;
        bool opened_ = false;
    };

}

#endif
