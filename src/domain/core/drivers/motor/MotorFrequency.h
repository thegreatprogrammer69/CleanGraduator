#ifndef CLEANGRADUATOR_MOTORFREQUENCY_H
#define CLEANGRADUATOR_MOTORFREQUENCY_H
#include <chrono>

namespace domain::common {
    class MotorFrequency {
    public:
        explicit MotorFrequency(float hz) noexcept
            : hz_(hz > 0.f ? hz : 0.f) {}

        float hz() const noexcept { return hz_; }

        bool isValid() const noexcept { return hz_ > 0.99f; }

        std::chrono::steady_clock::duration halfPeriod() const noexcept {
            using namespace std::chrono;

            if (hz_ <= std::numeric_limits<float>::epsilon())
                return steady_clock::duration::zero();

            const double seconds = 1.0 / (2.0 * hz_);
            return duration_cast<steady_clock::duration>(
                duration<double>(seconds)
            );
        }

        MotorFrequency withMaxHz(const float max_hz) const {
            float hz = hz_;
            if (hz > max_hz) hz = max_hz;
            return MotorFrequency(hz);
        }

    private:
        float hz_;
    };
}

#endif //CLEANGRADUATOR_MOTORFREQUENCY_H