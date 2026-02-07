#ifndef CLEANGRADUATOR_SLEEP_H
#define CLEANGRADUATOR_SLEEP_H
#include <chrono>

namespace infra::platform {
    void sleep(std::chrono::steady_clock::duration duration);
    void precise_sleep(std::chrono::steady_clock::duration duration);
}

#endif //CLEANGRADUATOR_SLEEP_H