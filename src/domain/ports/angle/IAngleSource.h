#ifndef CLEANGRADUATOR_IANGLESOURCE_H
#define CLEANGRADUATOR_IANGLESOURCE_H
#include "IAngleSink.h"


namespace domain::ports {
    struct IAngleSource {
        virtual ~IAngleSource() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void addSink(IAngleSink& sink) = 0;
        virtual void removeSink(IAngleSink& sink) = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESOURCE_H