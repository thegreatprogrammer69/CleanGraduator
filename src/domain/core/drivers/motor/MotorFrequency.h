#ifndef CLEANGRADUATOR_MOTORFREQUENCY_H
#define CLEANGRADUATOR_MOTORFREQUENCY_H
#include <chrono>

namespace domain::common {
    class MotorFrequency {
    public:
        explicit MotorFrequency(float hz = 0.f) noexcept
            : hz_(hz) {}

        // --- Copy ctor
        MotorFrequency(const MotorFrequency& other) noexcept
            : hz_(other.hz_.load(std::memory_order_relaxed)) {}

        // --- Copy assign
        MotorFrequency& operator=(const MotorFrequency& other) noexcept {
            if (this != &other) {
                hz_.store(
                    other.hz_.load(std::memory_order_relaxed),
                    std::memory_order_relaxed
                );
            }
            return *this;
        }

        // --- Move ctor
        MotorFrequency(MotorFrequency&& other) noexcept
            : hz_(other.hz_.load(std::memory_order_relaxed)) {}

        // --- Move assign
        MotorFrequency& operator=(MotorFrequency&& other) noexcept {
            if (this != &other) {
                hz_.store(
                    other.hz_.load(std::memory_order_relaxed),
                    std::memory_order_relaxed
                );
            }
            return *this;
        }

        float hz() const noexcept {
            return hz_.load(std::memory_order_relaxed);
        }

        void set(float hz) noexcept {
            hz_.store(hz, std::memory_order_relaxed);
        }

        bool isValid() const noexcept {
            return hz() > 0.99f;
        }

        void clampTo(float max_hz) noexcept
        {
            float current = hz_.load(std::memory_order_relaxed);

            while (current > max_hz) {
                if (hz_.compare_exchange_weak(current,max_hz)) {
                    break;
                }
            }
        }

        std::chrono::steady_clock::duration halfPeriod() const noexcept {
            using namespace std::chrono;

            const float value = hz_.load(std::memory_order_relaxed);

            if (value <= 0.f)
                return steady_clock::duration::zero();

            const double seconds = 1.0 / (2.0 * static_cast<double>(value));

            return duration_cast<steady_clock::duration>(
                duration<double>(seconds)
            );
        }

    private:
        std::atomic<float> hz_;
    };
}

#endif //CLEANGRADUATOR_MOTORFREQUENCY_H