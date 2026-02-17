#ifndef CLEANGRADUATOR_ICALIBRATIONRECORDER_H
#define CLEANGRADUATOR_ICALIBRATIONRECORDER_H
#include "CalibrationRecorderInput.h"
#include "domain/core/calibration/calculation/CalibrationPhase.h"
#include "domain/core/calibration/recording/CalibrationDataset.h"

namespace application::ports {
    struct ICalibrationRecorder {
        virtual ~ICalibrationRecorder() = default;

        virtual bool start(CalibrationRecorderInput) = 0;
        virtual void stop() = 0;

        virtual void setPhase(domain::common::CalibrationPhase phase) = 0;

        virtual domain::common::CalibrationDataset data() const = 0;

        virtual void resetSession() = 0;
        virtual bool isRecording() const = 0;
    };
}


#endif //CLEANGRADUATOR_ICALIBRATIONRECORDER_H