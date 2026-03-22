#ifndef CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONOBSERVER_H
#define CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONOBSERVER_H

#include "domain/core/calibration/result/ClibrationResultValidation.h"

namespace domain::ports {
class IClibrationResultValidationObserver {
public:
    virtual ~IClibrationResultValidationObserver() = default;
    virtual void onClibrationResultValidationUpdated(const domain::common::ClibrationResultValidation& validation) = 0;
};
}

#endif // CLEANGRADUATOR_ICLIBRATIONRESULTVALIDATIONOBSERVER_H
