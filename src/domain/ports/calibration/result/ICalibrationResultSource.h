#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTSOURCE_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTSOURCE_H
#include "ICalibrationResultObserver.h"

namespace domain::ports {
    class ICalibrationResultSource {
    public:
        virtual ~ICalibrationResultSource() = default;

        virtual const std::optional<common::CalibrationResult>& currentResult() const = 0;

        virtual void addObserver(ICalibrationResultObserver&) = 0;
        virtual void removeObserver(ICalibrationResultObserver&) = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTSOURCE_H