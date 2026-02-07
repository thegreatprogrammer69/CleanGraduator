#include "SessionClock.h"

using namespace domain::common;

namespace infra::clock {

SessionClock::SessionClock() = default;

void SessionClock::start() {
    startTime_ = std::chrono::steady_clock::now();
    started_ = true;
}

void SessionClock::stop() {
    started_ = false;
}

Timestamp SessionClock::now() const {
    if (!started_) {
        return Timestamp::fromDuration(std::chrono::steady_clock::duration::zero());
    }
    return Timestamp::fromDuration(std::chrono::steady_clock::now() - startTime_);
}

}
