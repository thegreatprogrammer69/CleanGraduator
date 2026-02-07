#ifndef CLEANGRADUATOR_PROCESSRUNNER_H
#define CLEANGRADUATOR_PROCESSRUNNER_H
#include "domain/ports/inbound/IProcessLifecycle.h"
#include "domain/ports/outbound/IProcessLifecycleObserver.h"


namespace infra::process {

class ProcessRunner : public IProcessLifecycleObserver {
public:
    explicit ProcessRunner(IProcessLifecycle& lifecycle);
    ~ProcessRunner() override;

    void onStateChanged(ProcessLifecycleState newState) override;

private:
    void onIdle();
    void onForward();
    void onBackward();
    void onStopping();

private:
    IProcessLifecycle &lifecycle_;
};

}

#endif //CLEANGRADUATOR_PROCESSRUNNER_H
