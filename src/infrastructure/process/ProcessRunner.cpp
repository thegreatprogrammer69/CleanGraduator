#include "ProcessRunner.h"


namespace infra::process {

ProcessRunner::ProcessRunner(IProcessLifecycle &lifecycle)
    : lifecycle_(lifecycle)
{
}

ProcessRunner::~ProcessRunner() {
}

void ProcessRunner::onStateChanged(ProcessLifecycleState newState) {
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
