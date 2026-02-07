#ifndef CLEANGRADUATOR_CHRONOSESSIONCLOCK_H
#define CLEANGRADUATOR_CHRONOSESSIONCLOCK_H

#include "domain/ports/outbound/IClock.h"
#include <chrono>

namespace infra::clock {

class SessionClock : public domain::ports::IClock {
public:
    SessionClock();

    void start();
    void stop();
    domain::common::Timestamp now() const override;

private:
    std::chrono::steady_clock::time_point startTime_;
    bool started_{false};
};

}

#endif // CLEANGRADUATOR_CHRONOSESSIONCLOCK_H
