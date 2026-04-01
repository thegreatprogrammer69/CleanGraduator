#include "sleep.h"
#include <windows.h>
#include <thread>
#include <immintrin.h>

namespace infra::platform {
    void sleep(std::chrono::steady_clock::duration duration) {
        std::this_thread::sleep_for(duration);
    }

    void precise_sleep(std::chrono::steady_clock::duration duration) {
        using namespace std::chrono;

        const auto start = steady_clock::now();
        const auto end = start + duration;

        while (steady_clock::now() < end) {
            _mm_pause();
        }
    }

    void auto_sleep(std::chrono::steady_clock::duration duration) {
        using namespace std::chrono;
        if (duration < 1ms) {
            precise_sleep(duration);
        }
        else {
            sleep(duration);
        }
    }
}