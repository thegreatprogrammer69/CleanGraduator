#ifndef CLEANGRADUATOR_CALIBRATIONCELL_H
#define CLEANGRADUATOR_CALIBRATIONCELL_H

#include <optional>


namespace domain::common {

    class CalibrationCell {
    public:
        CalibrationCell() = default;

        void setAngle(std::optional<float> v) { angle_ = std::move(v); }
        std::optional<float> angle() const { return angle_; }

    private:

        std::optional<float> angle_ = std::nullopt;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONCELL_H