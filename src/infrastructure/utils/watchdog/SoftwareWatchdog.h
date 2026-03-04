#ifndef CLEANGRADUATOR_SOFTWAREWATCHDOG_H
#define CLEANGRADUATOR_SOFTWAREWATCHDOG_H
#include <chrono>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace utils::watchdog {
    class SoftwareWatchdog {
    public:
        SoftwareWatchdog();
        ~SoftwareWatchdog();

        void start(std::chrono::milliseconds timeout);
        void stop();
        void kick();
        bool expired() const noexcept;

    private:
        using Clock = std::chrono::steady_clock;
        void run();

    private:
        std::chrono::milliseconds timeout_{0};
        Clock::time_point deadline_{};

        std::atomic<bool> expired_{false};
        bool running_{false};

        std::thread worker_;
        std::condition_variable cv_;
        std::mutex mutex_;
    };
}

#endif //CLEANGRADUATOR_SOFTWAREWATCHDOG_H