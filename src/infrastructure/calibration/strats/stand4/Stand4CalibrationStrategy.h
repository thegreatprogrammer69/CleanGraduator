#ifndef CLEANGRADUATOR_STAND4CALIBRATIONANGORITHM_H
#define CLEANGRADUATOR_STAND4CALIBRATIONANGORITHM_H
#include "domain/ports/calibration/strategy/ICalibrationStrategy.h"


#include <atomic>
#include <optional>

#include "domain/ports/drivers/motor/IMotorDriver.h"
#include "domain/ports/drivers/valve/IValveDriver.h"

#include "stand4logic.h"
#include "Stand4FrequencyCalculator.h"
#include "domain/ports/calibration/recording/ICalibrationRecorder.h"

namespace infra::calib::stand4 {
    class Stand4CalibrationStrategy final
        : public domain::ports::ICalibrationStrategy
    {
    public:
        Stand4CalibrationStrategy();

        void bind(
            domain::ports::IMotorDriver& motor,
            domain::ports::IValveDriver& valve,
            domain::ports::ICalibrationRecorder& recorder) override;

        void begin(const domain::common::CalibrationBeginContext& ctx) override;

        domain::ports::CalibrationDecisionType feed(const domain::common::CalibrationFeedContext& ctx) override;

        void end() override;

        bool isRunning() const override;

    private:
        enum class State {
            Idle,
            Preload,
            Forward,
            Backward,
            Finished,
            Fault
        };

    private:
        void updatePreload(const domain::common::CalibrationFeedContext& ctx);
        void updateForward(const domain::common::CalibrationFeedContext& ctx);
        void updateBackward(const domain::common::CalibrationFeedContext& ctx);

    private:
        domain::ports::IMotorDriver* motor_{nullptr};
        domain::ports::IValveDriver* valve_{nullptr};
        domain::ports::ICalibrationRecorder* recorder_{nullptr};

        Stand4FrequencyCalculator freq_calc_;

        std::atomic<State> state_{State::Idle};

        double p_preload_{0.0};
        double p_target_{0.0};
        double p_limit_{0.0};
        double dp_nominal_{0.0};

        double last_pressure_{0.0};
        double last_time_{0.0};
    };
}


#endif //CLEANGRADUATOR_STAND4CALIBRATIONANGORITHM_H