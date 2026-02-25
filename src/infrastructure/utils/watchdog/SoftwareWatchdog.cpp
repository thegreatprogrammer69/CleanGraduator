#include "SoftwareWatchdog.h"

utils::watchdog::SoftwareWatchdog::SoftwareWatchdog() = default;

utils::watchdog::SoftwareWatchdog::~SoftwareWatchdog() {
    stop();
}

void utils::watchdog::SoftwareWatchdog::start(std::chrono::milliseconds timeout) {
    stop(); // перезапуск безопасен

    {
        std::lock_guard<std::mutex> lock(mutex_);
        timeout_ = timeout;
        running_ = true;
        expired_.store(false, std::memory_order_release);
        deadline_ = Clock::now() + timeout_;
    }

    worker_ = std::thread([this] { run(); });
}

void utils::watchdog::SoftwareWatchdog::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_)
            return;
        running_ = false;
    }

    cv_.notify_all();

    if (worker_.joinable())
        worker_.join();
}

void utils::watchdog::SoftwareWatchdog::kick() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_)
        return;

    deadline_ = Clock::now() + timeout_;
    expired_.store(false, std::memory_order_release);
    cv_.notify_all();
}

bool utils::watchdog::SoftwareWatchdog::expired() const noexcept {
    return expired_.load(std::memory_order_acquire);
}

void utils::watchdog::SoftwareWatchdog::run() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (running_) {
        if (cv_.wait_until(lock, deadline_) == std::cv_status::timeout) {
            expired_.store(true, std::memory_order_release);
        }
    }
}
