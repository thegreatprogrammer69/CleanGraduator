#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTOBSERVER_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTOBSERVER_H
#include "domain/core/calibration/result/CalibrationResult.h"

namespace domain::ports {
    class ICalibrationResultObserver {
    public:
        virtual ~ICalibrationResultObserver() = default;
        virtual void onCalibrationResultUpdated(const common::CalibrationResult& result) = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTOBSERVER_H