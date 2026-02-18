#ifndef CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1_H
#define CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1_H
#include <atomic>

#include "CalibrationMotionController4Config.h"
#include "IPressurePointsTrackerObserver.h"
#include "PressurePointsTracker.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/motion_controller/ICalibrationMotionController.h"
#include "domain/ports/motor/IValveDriverObserver.h"
#include "domain/ports/motor/IMotorDriverObserver.h"
#include "domain/ports/telemetry/ITelemetrySource.h"
#include "infrastructure/calibration/motion_controller/CalibrationMotionControllerPorts.h"

namespace domain::ports {
    struct IValveDriver;
}

namespace infra::calib {
    class CalibrationMotionController4 final
        : public domain::ports::ICalibrationMotionController
        , public domain::ports::ITelemetrySource
        , domain::ports::IMotorDriverObserver
        , domain::ports::IValveDriverObserver
        , stand4::IPressurePointsTrackerObserver
    {
    public:
        CalibrationMotionController4(CalibrationMotionControllerPorts ports, CalibrationMotionController4Config config);
        ~CalibrationMotionController4() override;

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
        void onLimitsStateChanged(domain::common::MotorLimitsState) override;
        void onDirectionChanged(domain::common::MotorDirection) override;
        void onFault(const domain::common::MotorFault &fault) override;

        // IValveDriverObserver
        void onInputFlapOpened() override;
        void onOutputFlapOpened() override;
        void onFlapsClosed() override;

        // IPressurePointsTrackerObserver
        void onPointEntered(int index) override;
        void onPointExited(int index) override;

    public:

    private:
        fmt::Logger logger_;
        domain::ports::IMotorDriver& motor_driver_;
        domain::ports::IValveDriver& valve_driver_;
        CalibrationMotionController4Config config_;

    private:
        std::atomic_bool is_started_{false};
        std::atomic_bool is_motor_started_{false};
        std::atomic<domain::common::MotorDirection> current_direction_{domain::common::MotorDirection::Forward};
        stand4::PressurePointsTracker points_tracker_;


    };
}

#endif //CLEANGRADUATOR_CALIBRATIONMOTIONCONTROLLER1_H