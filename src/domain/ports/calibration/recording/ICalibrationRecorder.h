#ifndef CLEANGRADUATOR_IPRESSURERECORDER_H
#define CLEANGRADUATOR_IPRESSURERECORDER_H
#include <optional>

#include "domain/core/angle/AngleSourceId.h"
#include "domain/core/calibration/recording/TimeSeries.h"

namespace domain::common {
    enum class MotorDirection;
}

namespace domain::ports {

    struct ICalibrationRecorder {
        virtual ~ICalibrationRecorder() = default;

        virtual void beginSession(int point, common::MotorDirection direction) = 0;

        virtual void pushPressure(float time, float pressure) = 0;
        virtual void pushAngle(common::AngleSourceId id, float time, float angle) = 0;

        virtual void endSession() = 0;
    };

}

#endif //CLEANGRADUATOR_IPRESSURERECORDER_H