#ifndef CLEANGRADUATOR_IANGLESOURCE_H
#define CLEANGRADUATOR_IANGLESOURCE_H
#include "IAngleSourceObserver.h"


namespace domain::ports {
    struct IAngleSource {
        virtual ~IAngleSource() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const noexcept = 0;
        virtual void addObserver(IAngleSourceObserver&) = 0;
        virtual void removeObserver(IAngleSourceObserver&) = 0;
    };
}

#endif //CLEANGRADUATOR_IANGLESOURCE_H