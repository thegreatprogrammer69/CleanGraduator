#ifndef UNTITLED_G540LOGIC_H
#define UNTITLED_G540LOGIC_H
#include <chrono>
#include "IG540.h"

namespace infra::hardware::g540logic {
    // 1) Можно ли двигаться с учётом направления и лимитов
    inline bool canMove(G540Direction direction, G540LimitsState limits) {
        switch (direction) {
            case G540Direction::Neutral:
                return false;

            case G540Direction::Forward:
                return limits != G540LimitsState::End &&
                       limits != G540LimitsState::Both;

            case G540Direction::Backward:
                return limits != G540LimitsState::Begin &&
                       limits != G540LimitsState::Both;
        }

        return false;
    }

    // 2) Подсчёт half-period
    inline std::chrono::steady_clock::duration calculateHalfPeriod(int hz) {
        using namespace std::chrono;
        if (hz <= 0) hz = 1;
        return duration_cast<steady_clock::duration>(seconds(1) / (hz * 2));
    }

} // namespace g540logic

#endif //UNTITLED_G540LOGIC_H