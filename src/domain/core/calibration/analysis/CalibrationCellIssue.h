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
                return "Слишком мало значений давления в съёме";
            case CalibrationCellIssue::Type::TooFewAngleValues:
                return "Слишком мало значений углов в съёме";
            case CalibrationCellIssue::Type::FewPressureValues:
                return "Недостаточно значений давления в съёме";
            case CalibrationCellIssue::Type::FewAngleValues:
                return "Недостаточно значений углов в съёме";
            case CalibrationCellIssue::Type::PressureSeriesSizeMismatch:
                return "Несоответствие размеров временного ряда давления (размеры время/давление не совпадают)";
            case CalibrationCellIssue::Type::AngleSeriesSizeMismatch:
                return "Несоответствие размеров временного ряда угла (размеры время/угол не совпадают)";
            case CalibrationCellIssue::Type::PressurePointOutOfRange:
                return "Точка давления вне диапазона съёме";
            case CalibrationCellIssue::Type::InterpolatedTimeOutOfRange:
                return "Интерполированное время вне диапазона съёма углов";
        }

        return "Неизвестная ошибка";
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