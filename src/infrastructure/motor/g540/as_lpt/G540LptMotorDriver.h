#ifndef CLEANGRADUATOR_G540BOARDLPT_H
#define CLEANGRADUATOR_G540BOARDLPT_H
#include <memory>

#include "G540LptMotorDriverConfig.h"
#include "G540LptMotorDriverPorts.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/motor/IDualValveDriver.h"
#include "domain/ports/motor/IMotorDriver.h"


namespace infra::motors {

    class G540LptMotorDriver final : public domain::ports::IMotorDriver, public domain::ports::IDualValveDriver {
    public:
        G540LptMotorDriver(const G540LptMotorDriverPorts& ports,
                           const G540LptMotorDriverConfig& config);
        ~G540LptMotorDriver() override;

        // --- Lifecycle ---
        void start() override;
        void stop() override;
        void emergencyStop() override;

        // --- Motion config ---
        void setFrequency(int hz) override;
        void setDirection(domain::common::MotorDirection dir) override;

        // --- Limits ---
        domain::common::MotorLimitsState limits() const override;

        // --- Watchdog (если не используешь — можно no-op) ---
        void enableWatchdog(std::chrono::milliseconds timeout) override;
        void disableWatchdog() override;
        void feedWatchdog() override;

        // --- Fault state ---
        domain::common::MotorFault fault() const override;

        // --- Observers ---
        void addObserver(domain::ports::IMotorDriverObserver& o);
        void removeObserver(domain::ports::IMotorDriverObserver& o);

        // --- Extra (из старого кода) ---
        enum class FlapsState { CloseBoth, OpenInput, OpenOutput };
        void setFlapsState(FlapsState s);

        // IDualValveDriver
        void openInputFlap() override;
        void openOutputFlap() override;
        void closeFlaps() override;

    private:
        fmt::Logger logger_;

        struct G540LptImpl;
        std::unique_ptr<G540LptImpl> impl_;

        G540LptMotorDriverPorts ports_;
        G540LptMotorDriverConfig config_;

        int current_hz_;
        domain::common::MotorDirection current_dir_;
    };
}

#endif //CLEANGRADUATOR_G540BOARDLPT_H