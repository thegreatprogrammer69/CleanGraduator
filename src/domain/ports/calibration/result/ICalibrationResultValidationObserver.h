#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTVALIDATIONOBSERVER_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTVALIDATIONOBSERVER_H

#include "domain/core/calibration/validation/CalibrationResultValidation.h"

namespace domain::ports {
    class ICalibrationResultValidationObserver {
    public:
        virtual ~ICalibrationResultValidationObserver() = default;
        virtual void onCalibrationResultValidationUpdated(const common::CalibrationResultValidation& validation) = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTVALIDATIONOBSERVER_H
