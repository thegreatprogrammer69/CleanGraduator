#ifndef CLEANGRADUATOR_PRESSUREPOINTS_H
#define CLEANGRADUATOR_PRESSUREPOINTS_H
#include <vector>

namespace application::model {
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