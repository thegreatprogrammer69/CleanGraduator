#ifndef CLEANGRADUATOR_ICLIBRATIONRESULTOBSERVER_H
#define CLEANGRADUATOR_ICLIBRATIONRESULTOBSERVER_H

#include "domain/core/calibration/result/ClibrationResultValidation.h"

namespace domain::ports {

class IClibrationResultObserver {
public:
    virtual ~IClibrationResultObserver() = default;
    virtual void onClibrationResultValidationUpdated(const common::ClibrationResultValidation& validation) = 0;
};

} // namespace domain::ports

#endif // CLEANGRADUATOR_ICLIBRATIONRESULTOBSERVER_H
