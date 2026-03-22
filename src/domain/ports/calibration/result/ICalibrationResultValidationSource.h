#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTVALIDATIONSOURCE_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTVALIDATIONSOURCE_H

#include <optional>

#include "ICalibrationResultValidationObserver.h"

namespace domain::ports {
    class ICalibrationResultValidationSource {
    public:
        virtual ~ICalibrationResultValidationSource() = default;
        virtual const std::optional<common::CalibrationResultValidation>& currentValidation() const = 0;
        virtual void addObserver(ICalibrationResultValidationObserver& observer) = 0;
        virtual void removeObserver(ICalibrationResultValidationObserver& observer) = 0;
        virtual void requestRefresh() = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTVALIDATIONSOURCE_H
