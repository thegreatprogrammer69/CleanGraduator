#include "ProcessRunner.h"

#include "domain/ports/inbound/IProcessLifecycle.h"


namespace infra::process {

ProcessRunner::ProcessRunner(IProcessLifecycle &lifecycle)
    : lifecycle_(lifecycle)
{
    lifecycle_.subscribe(*this);
}

ProcessRunner::~ProcessRunner() {
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
