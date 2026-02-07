#ifndef CLEANGRADUATOR_IANGLESOURCE_H
#define CLEANGRADUATOR_IANGLESOURCE_H
#include "domain/ports/outbound/IAngleSink.h"


namespace domain::ports {
    class IAngleSource {
    public:
        virtual ~IAngleSource() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void addSink(IAngleSink& sink) = 0;
        virtual void removeSink(IAngleSink& sink) = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESOURCE_H