#include "sleep.h"
#include <windows.h>
#include <thread>

namespace infra::platform {
    void sleep(std::chrono::steady_clock::duration duration) {
        std::this_thread::sleep_for(duration);
    }

    void precise_sleep(std::chrono::steady_clock::duration duration) {
        using namespace std::chrono;

        // Переводим в 100-нс интервалы (Windows FILETIME unit)
        auto ns = duration_cast<nanoseconds>(duration).count();

        if (ns <= 0) {
            return;
        }

        // Относительное время — отрицательное значение
        LARGE_INTEGER dueTime;
        dueTime.QuadPart = -static_cast<LONGLONG>(ns / 100);

        HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
        if (!timer) {
            return; // можно логировать ошибку
        }

        if (SetWaitableTimer(timer, &dueTime, 0, nullptr, nullptr, FALSE)) {
            WaitForSingleObject(timer, INFINITE);
        }

        CloseHandle(timer);
    }
}