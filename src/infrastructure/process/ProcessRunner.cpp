#include "ProcessRunner.h"

#include "../../domain/ports/lifecycle/IProcessLifecycle.h"


namespace infra::process {

ProcessRunner::ProcessRunner(domain::ports::ILogger& logger, domain::ports::IProcessLifecycle &lifecycle)
    : lifecycle_(lifecycle), logger_(logger)
{
    lifecycle_.addObserver(*this);
}

ProcessRunner::~ProcessRunner() {
    lifecycle_.removeObserver(*this);
}

void ProcessRunner::onStateChanged(State newState) {

}

void ProcessRunner::onIdle() {
}

void ProcessRunner::onForward() {

}

void ProcessRunner::onBackward() {
}

void ProcessRunner::onStopping() {
}

}
