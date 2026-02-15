#ifndef CLEANGRADUATOR_ICALIBRATIONCALCULATOR_H
#define CLEANGRADUATOR_ICALIBRATIONCALCULATOR_H

namespace domain::common {
    struct CalibrationInput;
    class CalibrationResult;
}

namespace domain::ports {
    class ICalibrationCalculator {
    public:
        virtual ~ICalibrationCalculator() = default;
        virtual common::CalibrationResult calculate(const common::CalibrationInput& angles) const = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONCALCULATOR_H
