#ifndef CLEANGRADUATOR_STAND4CALIBRATIONANGORITHM_H
#define CLEANGRADUATOR_STAND4CALIBRATIONANGORITHM_H
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"


#include <atomic>
#include <optional>

#include "Stand4CalibrationStrategyConfig.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"

#include "stand4logic.h"
#include "Stand4FrequencyCalculator.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "infrastructure/calibration/strats/CalibrationStrategyPorts.h"
#include "infrastructure/calibration/tracking/IPressurePointsTrackerObserver.h"
#include "infrastructure/calibration/tracking/PressurePointsTracker.h"

namespace infra::calib::stand4 {
    class Stand4CalibrationStrategy final
        : public domain::ports::ICalibrationStrategy
        , tracking::IPressurePointsTrackerObserver
    {
    public:
        Stand4CalibrationStrategy(CalibrationStrategyPorts ports, Stand4CalibrationStrategyConfig config);
        ~Stand4CalibrationStrategy();
        void bind(
            domain::ports::IMotorDriver& motor,
            domain::ports::ICalibrationRecorder& recorder) override;

        domain::common::CalibrationStrategyVerdict begin(const domain::common::CalibrationStrategyBeginContext& ctx) override;

        domain::common::CalibrationStrategyVerdict feed(const domain::common::CalibrationStrategyFeedContext& ctx) override;

        void end() override;

        bool isRunning() const override;

        enum class State {
            Idle,
            Preload,
            Forward,
            Backward,
            Finished,
            Fault
        };

    protected:
        void onPressurePointsTrackerEvent(const tracking::PressurePointsTrackerEvent &) override;

    private:
        void updatePreload(const domain::common::CalibrationStrategyFeedContext& ctx);
        void updateForward(const domain::common::CalibrationStrategyFeedContext& ctx);
        void updateBackward(const domain::common::CalibrationStrategyFeedContext& ctx);

        void transition(State newState);

        void transitionToPreload();
        void transitionToForward();
        void transitionToBackward();
        void transitionToFinished();
        void transitionToFault();


    private:
        fmt::Logger logger_;

        domain::ports::IMotorDriver* motor_{nullptr};
        domain::ports::ICalibrationRecorder* recorder_{nullptr};

        Stand4FrequencyCalculator freq_calc_;
        tracking::PressurePointsTracker points_tracker_;

        std::atomic<State> state_{State::Idle};

        std::vector<float> pressure_points_;

        double p_preload_{0.0};
        double p_target_{0.0};
        double p_limit_{0.0};
        double dp_nominal_{0.0};

        double last_pressure_{0.0};
        double last_time_{0.0};
    };
}


#endif //CLEANGRADUATOR_STAND4CALIBRATIONANGORITHM_H