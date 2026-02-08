#ifndef UNTITLED_IPRESSURESENSOR_H
#define UNTITLED_IPRESSURESENSOR_H
#include "domain/ports/outbound/IPressureSink.h"


namespace domain::ports {
    struct IPressureSource {
        virtual ~IPressureSource() = default;
        virtual bool start() = 0;
        virtual void stop() = 0;
        virtual void addSink(IPressureSink& sink) = 0;
        virtual void removeSink(IPressureSink& sink) = 0;
    };
}

#endif //UNTITLED_IPRESSURESENSOR_H