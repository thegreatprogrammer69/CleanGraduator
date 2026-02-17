#ifndef CLEANGRADUATOR_TIMESTAMP_H
#define CLEANGRADUATOR_TIMESTAMP_H

#include <chrono>

namespace domain::common {

    class Timestamp {
    public:
        using clock = std::chrono::steady_clock;
        using duration = clock::duration;

        Timestamp() : value_(duration::zero()) {}

        static Timestamp fromDuration(duration d) {
            return Timestamp(d);
        }

        duration toDuration() const noexcept {
            return value_;
        }

        double asSeconds() const noexcept {
            return std::chrono::duration<double>(value_).count();
        }

        double asMilliseconds() const noexcept {
            return std::chrono::duration<double, std::milli>(value_).count();
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