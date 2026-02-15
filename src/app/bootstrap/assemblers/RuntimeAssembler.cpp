#include "RuntimeAssembler.h"

#include "infrastructure/clock/UptimeClock.h"
#include "infrastructure/process/ProcessLifecycle.h"

using namespace app::bootstrap;

RuntimeAssembly app::bootstrap::assembleRuntime() {
    RuntimeAssembly assembled;
    assembled.process_lifecycle = std::make_unique<infra::process::ProcessLifecycle>();
    assembled.session_clock = &assembled.process_lifecycle->clock();
    assembled.uptime_clock = std::make_unique<infra::clock::UptimeClock>();
    return assembled;
}
