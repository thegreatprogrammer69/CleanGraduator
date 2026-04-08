#ifndef CLEANGRADUATOR_STAND4CALIBRATIONSTRATEGY_H
#define CLEANGRADUATOR_STAND4CALIBRATIONSTRATEGY_H

#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"

#include <atomic>
#include <vector>

#include "Stand4CalibrationStrategyConfig.h"
#include "stand4logic.h"
#include "Stand4FrequencyCalculator.h"

#include "domain/core/calibration/common/CalibrationMode.h"
#include "domain/fmt/Logger.h"

#include "infrastructure/calibration/strats/CalibrationStrategyPorts.h"
#include "infrastructure/calibration/tracking/IPressurePointsTrackerObserver.h"
#include "infrastructure/calibration/tracking/PressurePointsTracker.h"

namespace infra::calib::stand4 {

class Stand4CalibrationStrategy final
    : public domain::ports::ICalibrationStrategy
    , public tracking::IPressurePointsTrackerObserver
{
public:
    using Verdict = domain::common::CalibrationStrategyVerdict;
    using BeginContext = domain::common::CalibrationStrategyBeginContext;
    using FeedContext  = domain::common::CalibrationStrategyFeedContext;

    Stand4CalibrationStrategy(
        CalibrationStrategyPorts ports,
        Stand4CalibrationStrategyConfig config);

    Verdict begin(const BeginContext& ctx) override;

    Verdict feed(const FeedContext& ctx) override;

    Verdict end() override;

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
    void onPressurePointsTrackerEvent(
        const tracking::PressurePointsTrackerEvent& ev) override;

private:

    /* ============================
       UPDATE METHODS
       ============================ */

    void updatePreload(const FeedContext& ctx, Verdict& v);
    void updateForward(const FeedContext& ctx, Verdict& v);
    void updateBackward(const FeedContext& ctx, Verdict& v);

    /* ============================
       STATE TRANSITIONS
       ============================ */

    void transition(State newState);

    void transitionToPreload(Verdict& v);
    void transitionToForward(Verdict& v);
    void transitionToBackward(Verdict& v);
    void transitionToFinished(Verdict& v);
    void transitionToFault(Verdict& v);

private:

    fmt::Logger logger_;

    Stand4FrequencyCalculator freq_calc_;

    tracking::PressurePointsTracker points_tracker_;

    std::atomic<State> state_{State::Idle};

    std::vector<float> pressure_points_;

    double p_preload_{0.0};
    double p_target_{0.0};
    double p_limit_{0.0};
    double dp_nominal_{0.0};
    double p_backward_start_{0.0};
    double p_backward_target_{0.0};

    double last_pressure_{0.0};
    double last_time_{0.0};
    domain::common::CalibrationMode calibration_mode_{
        domain::common::CalibrationMode::Full
    };

    // команды, накопленные из tracker callbacks
    std::vector<Verdict::Command> pending_;
};

} // namespace infra::calib::stand4

#endif
