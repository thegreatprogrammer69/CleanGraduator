#ifndef CLEANGRADUATOR_IPROCESSLIFECYCLEOBSERVER_H
#define CLEANGRADUATOR_IPROCESSLIFECYCLEOBSERVER_H
#include "domain/core/process/ProcessLifecycleState.h"


namespace domain::ports {
    class IProcessLifecycleObserver {
    public:
        virtual ~IProcessLifecycleObserver() = default;
        virtual void onStateChanged(common::ProcessLifecycleState newState) = 0;
    };
}

#endif //CLEANGRADUATOR_IPROCESSLIFECYCLEOBSERVER_H