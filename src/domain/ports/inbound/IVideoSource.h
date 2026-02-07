#ifndef CLEANGRADUATOR_ICAMERASTREAM_H
#define CLEANGRADUATOR_ICAMERASTREAM_H
#include "domain/ports/outbound/IVideoSink.h"


namespace domain::ports {
    class IVideoSource {
    public:
        virtual ~IVideoSource() = default;
        virtual bool start() = 0;
        virtual void stop() = 0;
        virtual void addSink(IVideoSink& sink) = 0;
        virtual void removeSink(IVideoSink& sink) = 0;
    };
}

#endif //CLEANGRADUATOR_ICAMERASTREAM_H