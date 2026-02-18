#ifndef CLEANGRADUATOR_PRESSUREPOINTS_H
#define CLEANGRADUATOR_PRESSUREPOINTS_H
#include <vector>

namespace application::model {
    struct GaugePressurePoints {
        explicit GaugePressurePoints(std::vector<double> points)
            : value(std::move(points))
        {}
        GaugePressurePoints() = default;
        std::vector<double> value;
        bool isCorrect() const {
            return value.size() > 1;
        }
        bool operator==(const GaugePressurePoints &other) const {  return value == other.value; }
    };
}

#endif //CLEANGRADUATOR_PRESSUREPOINTS_H