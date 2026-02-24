#ifndef CLEANGRADUATOR_MOTORFREQUENCY_H
#define CLEANGRADUATOR_MOTORFREQUENCY_H
#include <chrono>

namespace domain::common {
    class MotorFrequency {
    public:
        explicit MotorFrequency(float hz) noexcept
            : hz_(hz > 0.f ? hz : 0.f) {}

        float hz() const noexcept { return hz_; }

        std::chrono::steady_clock::duration halfPeriod() const noexcept {
            using namespace std::chrono;

            if (hz_ <= std::numeric_limits<float>::epsilon())
                return steady_clock::duration::zero();

            const double seconds = 1.0 / (2.0 * hz_);
            return duration_cast<steady_clock::duration>(
                duration<double>(seconds)
            );
        }

    private:
        float hz_;
    };
}

#endif //CLEANGRADUATOR_MOTORFREQUENCY_H