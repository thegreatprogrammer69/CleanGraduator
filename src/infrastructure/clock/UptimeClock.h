#ifndef CLEANGRADUATOR_RUNTIMECLOCK_H
#define CLEANGRADUATOR_RUNTIMECLOCK_H

#include "domain/ports/outbound/IClock.h"
#include <chrono>

namespace infra::clock {

    class UptimeClock final : public domain::ports::IClock {
    public:
        UptimeClock();

        domain::common::Timestamp now() const override;

    private:
        std::chrono::steady_clock::time_point startTime_;
    };

}

#endif // CLEANGRADUATOR_RUNTIMECLOCK_H
