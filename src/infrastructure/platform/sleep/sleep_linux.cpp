#include "sleep.h"
#include <thread>

namespace infra::platform {
    void sleep(std::chrono::steady_clock::duration duration) {
         std::this_thread::sleep_for(duration);
    }

    void precise_sleep(std::chrono::steady_clock::duration duration) {
        using namespace std::chrono;

        // переводим duration в наносекунды
        auto ns = duration_cast<nanoseconds>(duration);

        timespec req{};
        req.tv_sec  = static_cast<time_t>(ns.count() / 1'000'000'000);
        req.tv_nsec = static_cast<long>(ns.count() % 1'000'000'000);

        // nanosleep может быть прерван сигналом → нужно досыпать
        while (nanosleep(&req, &req) == -1) {
            if (errno != EINTR) {
                break; // реальная ошибка
            }
            // если EINTR — req уже содержит оставшееся время
        }
    }
}