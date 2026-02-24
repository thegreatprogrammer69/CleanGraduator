#ifndef CLEANGRADUATOR_G540LPTMOTORDRIVER_H
#define CLEANGRADUATOR_G540LPTMOTORDRIVER_H
#include <memory>

#include "G540LptMotorDriverConfig.h"
#include "domain/fmt/Logger.h"
#include "domain/ports/drivers/motor/IMotorDriver.h"
#include "domain/core/drivers/motor/MotorFrequency.h"
#include "infrastructure/motor/MotorDriverPorts.h"
#include "infrastructure/motor/notifier/MotorDriverNotifier.h"
#include "infrastructure/platform/lpt/LptPort.h"
#include "infrastructure/utils/thread/ThreadWorker.h"
#include "infrastructure/utils/atomic/AtomicStruct.h"

namespace infra::motor {
    class G540LPTMotorDriver final : public domain::ports::IMotorDriver {
    public:
        G540LPTMotorDriver(MotorDriverPorts, motors::G540LptMotorDriverConfig);
        ~G540LPTMotorDriver() override;

        bool initialize() override;
        bool start() override;
        void stop() override;
        domain::common::MotorDriverState state() const override;

        void emergencyStop() override;
        bool resetError() override;
        domain::common::MotorDriverError error() const override;
        void enableWatchdog(std::chrono::milliseconds timeout) override;
        void disableWatchdog() override;

        void setFrequency(domain::common::MotorFrequency frequency) override;
        domain::common::MotorFrequency frequency() const override;
        domain::common::MotorFrequencyLimits frequencyLimits() const override;

        void setDirection(domain::common::MotorDirection dir) override;
        domain::common::MotorDirection direction() const override;

        void setFlapsState(domain::common::MotorFlapsState state) override;
        domain::common::MotorFlapsState flapsState() const override;

        domain::common::MotorLimitsState limits() const override;

        void addObserver(domain::ports::IMotorDriverObserver &) override;
        void removeObserver(domain::ports::IMotorDriverObserver &) override;

    private:
        fmt::Logger logger_;
        motors::G540LptMotorDriverConfig config_;

        utils::thread::ThreadWorker thread_worker_;
        MotorDriverNotifier notifier_;


        std::atomic<domain::common::MotorDriverState> state_;
        utils::atomic::AtomicStruct<domain::common::MotorDriverError> error_;
        utils::atomic::AtomicStruct<domain::common::MotorFrequency> frequency_;

        platform::LptPort lpt_port_;
    };
}

#endif //CLEANGRADUATOR_G540LPTMOTORDRIVER_H