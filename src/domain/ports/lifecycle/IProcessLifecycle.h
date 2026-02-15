#ifndef CLEANGRADUATOR_IPROCESSLIFECYCLE_H
#define CLEANGRADUATOR_IPROCESSLIFECYCLE_H
#include "domain/ports/lifecycle/IProcessLifecycleObserver.h"


namespace domain::ports {
    struct IProcessLifecycle {
        virtual ~IProcessLifecycle() = default;

        virtual bool canStart() const = 0;
        virtual bool canStop() const = 0;

        virtual void markIdle() = 0;
        virtual void markForward() = 0;
        virtual void markBackward() = 0;
        virtual void markStopping() = 0;

        virtual common::ProcessLifecycleState state() const = 0;

        virtual void subscribe(IProcessLifecycleObserver&) = 0;
    };
}


#endif //CLEANGRADUATOR_IPROCESSLIFECYCLE_H