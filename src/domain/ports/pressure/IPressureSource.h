#ifndef UNTITLED_IPRESSURESENSOR_H
#define UNTITLED_IPRESSURESENSOR_H
#include "IPressureSourceObserver.h"


namespace domain::ports {
    struct IPressureSource {
        virtual ~IPressureSource() = default;
        virtual bool start() = 0;
        virtual void stop() = 0;
        virtual void addObserver(IPressureSourceObserver& observer) = 0;
        virtual void removeObserver(IPressureSourceObserver& observer) = 0;
    };
}

#endif //UNTITLED_IPRESSURESENSOR_H