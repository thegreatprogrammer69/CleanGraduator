#ifndef CLEANGRADUATOR_CALIBRATIONCELLISSUE_H
#define CLEANGRADUATOR_CALIBRATIONCELLISSUE_H
#include <string>

#include "CalibrationIssueSeverity.h"
#include "domain/core/calibration/result/CalibrationCellKey.h"

namespace domain::common {
    struct CalibrationCellIssue
    {
        enum class Type {
            // input validation
            TooFewPressureValues,
            TooFewAngleValues,

            FewPressureValues,
            FewAngleValues,

            PressureSeriesSizeMismatch,
            AngleSeriesSizeMismatch,

            // calculation
            PressurePointOutOfRange,
            InterpolatedTimeOutOfRange
        };

        Type type;
        CalibrationIssueSeverity severity;
        std::string message;
    };

    inline std::string to_string(CalibrationCellIssue::Type type)
    {
        switch (type) {
            case CalibrationCellIssue::Type::TooFewPressureValues:
                return "Too few pressure values in series";
            case CalibrationCellIssue::Type::TooFewAngleValues:
                return "Too few angle values in series";
            case CalibrationCellIssue::Type::FewPressureValues:
                return "Few pressure values in series";
            case CalibrationCellIssue::Type::FewAngleValues:
                return "Few angle values in series";
            case CalibrationCellIssue::Type::PressureSeriesSizeMismatch:
                return "Pressure series time/value size mismatch";
            case CalibrationCellIssue::Type::AngleSeriesSizeMismatch:
                return "Angle series time/value size mismatch";
            case CalibrationCellIssue::Type::PressurePointOutOfRange:
                return "Pressure point is outside pressure series range";
            case CalibrationCellIssue::Type::InterpolatedTimeOutOfRange:
                return "Interpolated time is outside angle series range";
        }

        return "Unknown calibration issue";
    }

    inline CalibrationCellIssue make_calibration_issue(
        CalibrationCellIssue::Type type,
        CalibrationIssueSeverity severity)
    {
        return CalibrationCellIssue{
            type,
            severity,
            to_string(type)
        };
    }

} // namespace domain::common

#endif //CLEANGRADUATOR_CALIBRATIONCELLISSUE_H