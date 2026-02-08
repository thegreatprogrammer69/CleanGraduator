#ifndef CLEANGRADUATOR_CALIBRATIONRESULT_H
#define CLEANGRADUATOR_CALIBRATIONRESULT_H
#include <optional>
#include <vector>
#include "ResultAngle.h"
#include "domain/core/measurement/PressurePoints.h"

namespace domain::common {

class Result {
public:
    Result(std::vector<ResultAngle> angles, double angle_range, double no_lin)
        : angles_(std::move(angles))
        , angle_range_(angle_range), no_lin_(no_lin)
    {}
    std::optional<ResultAngle> operator[](std::size_t index) const {
        if (index < angles_.size()) return angles_[index];
        return std::nullopt;
    }
    std::size_t size() const { return angles_.size(); }
    double angleRange() const { return angle_range_; }
    double noLin() const { return no_lin_; }

private:
    std::vector<ResultAngle> angles_;
    double angle_range_;
    double no_lin_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULT_H