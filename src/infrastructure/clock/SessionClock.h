#ifndef CLEANGRADUATOR_CHRONOSESSIONCLOCK_H
#define CLEANGRADUATOR_CHRONOSESSIONCLOCK_H

#include "domain/ports/clock/ISessionClock.h"
#include <chrono>

namespace infra::clock {

class SessionClock : public domain::ports::ISessionClock {
public:
    SessionClock();

    void start() override;
    void stop() override;
    domain::common::Timestamp now() const override;

private:
    std::chrono::steady_clock::time_point startTime_;
    bool started_{false};
};

}

#endif // CLEANGRADUATOR_CHRONOSESSIONCLOCK_H
