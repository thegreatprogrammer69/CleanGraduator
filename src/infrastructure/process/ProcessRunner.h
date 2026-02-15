#ifndef CLEANGRADUATOR_PROCESSRUNNER_H
#define CLEANGRADUATOR_PROCESSRUNNER_H
#include "domain/fmt/Logger.h"
#include "domain/ports/outbound/IProcessLifecycleObserver.h"


namespace domain::ports {
    struct ILogger;
    class IProcessLifecycle;
}

namespace infra::process {

class ProcessRunner : public domain::ports::IProcessLifecycleObserver {
    using State = domain::common::ProcessLifecycleState;
    using IProcessLifecycle = domain::ports::IProcessLifecycle;

public:
    explicit ProcessRunner(domain::ports::ILogger& logger, IProcessLifecycle& lifecycle);
    ~ProcessRunner() override;

    void onStateChanged(State newState) override;

private:
    void onIdle();
    void onForward();
    void onBackward();
    void onStopping();

private:
    IProcessLifecycle &lifecycle_;
    fmt::Logger logger_;
};

}

#endif //CLEANGRADUATOR_PROCESSRUNNER_H
