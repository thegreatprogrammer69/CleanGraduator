#ifndef CLEANGRADUATOR_DDPRESSUREPOINTS_H
#define CLEANGRADUATOR_DDPRESSUREPOINTS_H
#include <vector>

#include "domain/core/measurement/Pressure.h"

namespace domain::common {
    struct PressurePoints {
        explicit PressurePoints(std::vector<Pressure> points)
            : value(std::move(points))
        {}
        PressurePoints() = default;
        std::vector<Pressure> value;
        bool isCorrect() const {
            return value.size() > 1;
        }
        bool operator==(const PressurePoints &other) const {  return value == other.value; }
    };
}

#endif //CLEANGRADUATOR_DDPRESSUREPOINTS_H