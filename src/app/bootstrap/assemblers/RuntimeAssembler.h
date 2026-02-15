#ifndef CLEANGRADUATOR_RUNTIMEASSEMBLER_H
#define CLEANGRADUATOR_RUNTIMEASSEMBLER_H

#include <memory>

namespace domain::ports {
    struct IClock;
    struct IProcessLifecycle;
}

namespace app::bootstrap {

struct RuntimeAssembly {
    std::unique_ptr<domain::ports::IProcessLifecycle> process_lifecycle;
    std::unique_ptr<domain::ports::IClock> uptime_clock;
    domain::ports::IClock* session_clock;
};

RuntimeAssembly assembleRuntime();

}

#endif //CLEANGRADUATOR_RUNTIMEASSEMBLER_H
