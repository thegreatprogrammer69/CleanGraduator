#ifndef CLEANGRADUATOR_PRESSUREPOINTS_H
#define CLEANGRADUATOR_PRESSUREPOINTS_H
#include <vector>

namespace domain::common {
    struct GaugePressurePoints {
        explicit GaugePressurePoints(std::vector<double> points)
            : points(std::move(points))
        {}
        GaugePressurePoints() = default;
        std::vector<double> points;
        bool isCorrect() const {
            return points.size() > 1;
        }
        bool operator==(const GaugePressurePoints &other) const {  return points == other.points; }
    };
}

#endif //CLEANGRADUATOR_PRESSUREPOINTS_H