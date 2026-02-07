#ifndef CLEANGRADUATOR_TIMESTAMP_H
#define CLEANGRADUATOR_TIMESTAMP_H

#include <chrono>

namespace domain::common {

class Timestamp {
public:
    using clock = std::chrono::steady_clock;
    using duration = clock::duration;

    Timestamp() : value_(std::chrono::steady_clock::duration::zero()) {}

    static Timestamp fromDuration(duration d) {
        return Timestamp(d);
    }


    duration toDuration() const noexcept {
        return value_;
    }

    bool operator<(const Timestamp& other) const noexcept {
        return value_ < other.value_;
    }

    bool operator==(const Timestamp& other) const noexcept {
        return value_ == other.value_;
    }

private:
    explicit Timestamp(duration d) : value_(d) {}

    duration value_;
};

}

#endif //CLEANGRADUATOR_TIMESTAMP_H