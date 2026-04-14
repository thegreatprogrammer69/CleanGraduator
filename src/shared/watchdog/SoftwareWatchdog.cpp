#include "SoftwareWatchdog.h"

#include <iostream>
#include <stdexcept>

namespace shared::watchdog {

    SoftwareWatchdog::SoftwareWatchdog() = default;

    SoftwareWatchdog::~SoftwareWatchdog() {
        stop();
    }

    void SoftwareWatchdog::start(std::chrono::milliseconds timeout) {
        if (timeout <= std::chrono::milliseconds::zero()) {
            throw std::invalid_argument("SoftwareWatchdog timeout must be positive");
        }

        stop(); // Гарантируем корректный сброс, если start() вызван повторно

        std::lock_guard<std::mutex> lock(mutex_);
        timeout_ = timeout;
        deadline_ = Clock::now() + timeout_;
        running_ = true;
        expired_.store(false, std::memory_order_release);

        try {
            worker_ = std::thread([this]() {
                try {
                    run();
                } catch (const std::exception& e) {
                    std::cerr << "SoftwareWatchdog worker failed: " << e.what() << std::endl;
                    std::lock_guard<std::mutex> inner_lock(mutex_);
                    running_ = false;
                    expired_.store(false, std::memory_order_release);
                } catch (...) {
                    std::cerr << "SoftwareWatchdog worker failed: unknown exception" << std::endl;
                    std::lock_guard<std::mutex> inner_lock(mutex_);
                    running_ = false;
                    expired_.store(false, std::memory_order_release);
                }
            });
        } catch (...) {
            running_ = false;
            expired_.store(false, std::memory_order_release);
            throw;
        }
    }

    void SoftwareWatchdog::stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                return;
            }
            running_ = false;
            // Если остановлен, expired всегда false, как ты и просил
            expired_.store(false, std::memory_order_release);
        }

        cv_.notify_one();

        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void SoftwareWatchdog::feed() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_) {
            deadline_ = Clock::now() + timeout_;
            expired_.store(false, std::memory_order_release);
            cv_.notify_one(); // Будим поток, чтобы он пересчитал wait_until
        }
    }

    bool SoftwareWatchdog::expired() const noexcept {
        // Метод const noexcept, так что просто читаем атомик.
        // Всю работу со временем делает worker_.
        return expired_.load(std::memory_order_acquire);
    }

    void SoftwareWatchdog::run() {
        std::unique_lock<std::mutex> lock(mutex_);

        while (running_) {
            // Засыпаем до дедлайна. Если будет feed(), поток проснется
            // (из-за notify_one), проверит условия и уснет с новым deadline_
            cv_.wait_until(lock, deadline_);

            if (!running_) {
                break;
            }

            // Защита от spurious wakeups (ложных пробуждений)
            if (Clock::now() >= deadline_) {
                // Таймаут вышел! Ставим атомик в true
                expired_.store(true, std::memory_order_release);

                // Засыпаем и ждём, пока нас не покормят (feed) или не остановят (stop)
                cv_.wait(lock, [this]() {
                    return !running_ || Clock::now() < deadline_;
                });
            }
        }
    }

} // namespace shared::watchdog
