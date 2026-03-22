#ifndef CLEANGRADUATOR_IPRESSURERECORDER_H
#define CLEANGRADUATOR_IPRESSURERECORDER_H
#include <map>
#include <optional>
#include <unordered_map>
#include "domain/core/calibration/recording/AngleSample.h"
#include "domain/core/calibration/recording/CalibrationSession.h"
#include "domain/core/calibration/recording/CalibrationSessionId.h"
#include "domain/core/calibration/recording/PressureSample.h"
#include "domain/core/calibration/common/CalibrationLayout.h"
#include "domain/core/calibration/recording/CalibrationRecordingContext.h"

namespace domain::common {
    enum class MotorDirection;
}

namespace domain::ports {
    struct ICalibrationRecorderObserver;

    using CalibrationAngleCounts = std::unordered_map<
        common::SourceId,
        std::map<common::MotorDirection, int>>;

    struct ICalibrationRecorder {
        virtual ~ICalibrationRecorder() = default;

        // recording lifecycle
        virtual void startRecording(common::CalibrationRecordingContext) = 0;
        virtual void stopRecording() = 0;

        // session lifecycle
        virtual void beginSession(common::CalibrationSessionId) = 0;
        virtual void record(const common::PressureSample&) = 0;
        virtual void record(const common::AngleSample&) = 0;
        virtual void endSession() = 0;

        // read
        virtual std::vector<common::CalibrationSessionId> sessions() const = 0;
        virtual std::optional<common::CalibrationSession>
        session(common::CalibrationSessionId id) const = 0;
        virtual CalibrationAngleCounts angleCounts() const = 0;

        // observing
        virtual void addObserver(ICalibrationRecorderObserver& observer) = 0;
        virtual void removeObserver(ICalibrationRecorderObserver& observer) = 0;
    };

}
#endif //CLEANGRADUATOR_IPRESSURERECORDER_H