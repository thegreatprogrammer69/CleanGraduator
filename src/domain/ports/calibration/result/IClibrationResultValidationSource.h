#ifndef CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONSOURCE_H
#define CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONSOURCE_H

#include <optional>

#include "IClibrationResultValidationObserver.h"

namespace domain::ports {
class IClibrationResultValidationSource {
public:
    virtual ~IClibrationResultValidationSource() = default;
    virtual const std::optional<domain::common::ClibrationResultValidation>& currentValidation() const = 0;
    virtual void addObserver(IClibrationResultValidationObserver& observer) = 0;
    virtual void removeObserver(IClibrationResultValidationObserver& observer) = 0;
};
}

#endif // CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONSOURCE_H
