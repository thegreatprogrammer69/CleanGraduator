#ifndef CLEANGRADUATOR_ICAMERASTREAM_H
#define CLEANGRADUATOR_ICAMERASTREAM_H

namespace domain::events {
    class IEventListener;
}

namespace domain::ports {
    struct IVideoSource {
        virtual ~IVideoSource() = default;
        virtual void open() = 0;
        virtual void close() = 0;
    };
}

#endif //CLEANGRADUATOR_ICAMERASTREAM_H