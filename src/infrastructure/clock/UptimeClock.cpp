#include "UptimeClock.h"

using namespace domain::common;

namespace infra::clock {

    UptimeClock::UptimeClock()
        : startTime_(std::chrono::steady_clock::now())
    {
    }

    Timestamp UptimeClock::now() const {
        return Timestamp::fromDuration(
            std::chrono::steady_clock::now() - startTime_
        );
    }

}
