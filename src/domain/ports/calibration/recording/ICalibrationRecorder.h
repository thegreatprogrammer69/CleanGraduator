#ifndef CLEANGRADUATOR_IPRESSURERECORDER_H
#define CLEANGRADUATOR_IPRESSURERECORDER_H
#include "domain/core/calibration/recording/AngleSample.h"
#include "domain/core/calibration/recording/CalibrationSession.h"
#include "domain/core/calibration/recording/CalibrationSessionId.h"
#include "domain/core/calibration/recording/PressureSample.h"

namespace domain::common {
    enum class MotorDirection;
}

namespace domain::ports {

    struct ICalibrationRecorder {
        virtual ~ICalibrationRecorder() = default;

        // write
        virtual void beginSession(common::CalibrationSessionId id) = 0;
        virtual void record(const common::PressureSample&) = 0;
        virtual void record(const common::AngleSample&) = 0;
        virtual void endSession() = 0;

        // read
        virtual std::vector<common::CalibrationSessionId> sessions() const = 0;
        virtual std::optional<common::CalibrationSession> session(common::CalibrationSessionId id) const = 0;
    };

}

#endif //CLEANGRADUATOR_IPRESSURERECORDER_H