#ifndef CLEANGRADUATOR_CALIBRATIONCELL_H
#define CLEANGRADUATOR_CALIBRATIONCELL_H

namespace domain::common {

    class CalibrationCell
    {
    public:

        CalibrationCell() = default;

        void setAngle(float v) { angle_ = v; }
        float angle() const { return angle_; }

    private:

        float angle_ = 0.0f;
    };

}

#endif //CLEANGRADUATOR_CALIBRATIONCELL_H