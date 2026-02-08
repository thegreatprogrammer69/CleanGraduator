#ifndef CLEANGRADUATOR_CALIBRATIONCELL_H
#define CLEANGRADUATOR_CALIBRATIONCELL_H
#include <string>

#include "CalibrationCellSeverity.h"

namespace domain::common {

class CalibrationCell {
public:
    explicit CalibrationCell(double angle)
        : angle_(angle), severity_(CalibrationCellSeverity::None) {}

    double angle() const { return angle_; }
    CalibrationCellSeverity severity() const { return severity_; }
    const std::string& message() const { return message_; }

    void mark(CalibrationCellSeverity severity, std::string message) {
        severity_ = severity;
        message_ = std::move(message);
    }

private:
    double angle_;
    CalibrationCellSeverity severity_;
    std::string message_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONCELL_H