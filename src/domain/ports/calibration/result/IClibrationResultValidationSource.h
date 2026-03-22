#ifndef CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONSOURCE_H
#define CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONSOURCE_H

#include <optional>

#include "IClibrationResultObserver.h"

namespace domain::ports {

class IClibrationResultValidationSource {
public:
    virtual ~IClibrationResultValidationSource() = default;

    virtual const std::optional<common::ClibrationResultValidation>& currentValidation() const = 0;
    virtual void addObserver(IClibrationResultObserver& observer) = 0;
    virtual void removeObserver(IClibrationResultObserver& observer) = 0;
    virtual void revalidate() = 0;
    virtual void setKuModeEnabled(bool enabled) = 0;
    virtual bool kuModeEnabled() const = 0;
};

} // namespace domain::ports

#endif // CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONSOURCE_H
