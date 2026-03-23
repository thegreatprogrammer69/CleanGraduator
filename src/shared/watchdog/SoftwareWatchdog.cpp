#include "SoftwareWatchdog.h"

shared::watchdog::SoftwareWatchdog::SoftwareWatchdog() = default;

shared::watchdog::SoftwareWatchdog::~SoftwareWatchdog() {
    stop();
}

void shared::watchdog::SoftwareWatchdog::start(std::chrono::milliseconds timeout) {
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

void shared::watchdog::SoftwareWatchdog::stop() {
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

void shared::watchdog::SoftwareWatchdog::feed() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!running_)
        return;

    deadline_ = Clock::now() + timeout_;
    expired_.store(false, std::memory_order_release);
    cv_.notify_all();
}

bool shared::watchdog::SoftwareWatchdog::expired() const noexcept {
    return expired_.load(std::memory_order_acquire);
}

void shared::watchdog::SoftwareWatchdog::run() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (running_) {
        if (cv_.wait_until(lock, deadline_) == std::cv_status::timeout) {
            expired_.store(true, std::memory_order_release);
        }
    }
}
