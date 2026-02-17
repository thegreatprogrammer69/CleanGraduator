#ifndef CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1_H
#define CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1_H
#include "CalibrationMotionController4Config.h"
#include "domain/ports/calibration/motion_controller/ICalibrationMotionController.h"
#include "domain/ports/motor/IMotorDriverObserver.h"
#include "domain/ports/telemetry/ITelemetrySource.h"
#include "infrastructure/calibration/motion_controller/CalibrationMotionControllerPorts.h"

namespace domain::ports {
    struct IDualValveDriver;
}

namespace infra::calib {
    class CalibrationMotionController4 final
        : public domain::ports::ICalibrationMotionController
        , domain::ports::IMotorDriverObserver
        , public domain::ports::ITelemetrySource
    {
    public:
        CalibrationMotionController4(CalibrationMotionControllerPorts ports, CalibrationMotionController4Config config);

        //ICalibrationMotionController
        void start(domain::ports::CalibrationMotionInput input) override;
        void stop() override;
        void abort() override;
        bool isRunning() const override;
        void addObserver(domain::ports::ICalibrationMotionObserver &) override;
        void removeObserver(domain::ports::ICalibrationMotionObserver &) override;

    public:
        // ITelemetrySource
        std::vector<domain::ports::TelemetryField> telemetrySchema() const override;
        std::vector<domain::ports::TelemetryValue> telemetryValues() const override;

    protected:
        // IMotorDriverObserver
        void onStarted() override;
        void onStopped() override;
        void onFault(const domain::common::MotorFault &fault) override;

    private:

    };
}

#endif //CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1_H