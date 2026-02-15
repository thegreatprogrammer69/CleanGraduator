#ifndef CLEANGRADUATOR_PROCESSRUNNER_H
#define CLEANGRADUATOR_PROCESSRUNNER_H
#include "domain/fmt/Logger.h"
#include "../../domain/ports/lifecycle/IProcessLifecycleObserver.h"

namespace domain::ports {
    struct IProcessLifecycle;
}

namespace infra::process {

class ProcessRunner : public domain::ports::IProcessLifecycleObserver {
    using State = domain::common::ProcessLifecycleState;

public:
    explicit ProcessRunner(domain::ports::ILogger& logger, domain::ports::IProcessLifecycle& lifecycle);
    ~ProcessRunner() override;

    void onStateChanged(State newState) override;

private:
    void onIdle();
    void onForward();
    void onBackward();
    void onStopping();

private:
    domain::ports::IProcessLifecycle &lifecycle_;
    fmt::Logger logger_;
};

}

#endif //CLEANGRADUATOR_PROCESSRUNNER_H
