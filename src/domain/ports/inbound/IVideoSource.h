#ifndef CLEANGRADUATOR_ICAMERASTREAM_H
#define CLEANGRADUATOR_ICAMERASTREAM_H
#include "domain/ports/outbound/IVideoSink.h"


namespace domain::ports {
    struct IVideoSource {
        virtual ~IVideoSource() = default;
        virtual void open() = 0;
        virtual void close() = 0;
        virtual void addSink(IVideoSink& sink) = 0;
        virtual void removeSink(IVideoSink& sink) = 0;
    };
}

#endif //CLEANGRADUATOR_ICAMERASTREAM_H