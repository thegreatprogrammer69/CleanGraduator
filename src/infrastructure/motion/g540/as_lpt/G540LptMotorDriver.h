#ifndef CLEANGRADUATOR_G540LPTMOTORDRIVER_H
#define CLEANGRADUATOR_G540LPTMOTORDRIVER_H

#include <atomic>
#include <memory>

#include "G540LptMotorDriverConfig.h"
#include "../MotorDriverPorts.h"
#include "domain/core/drivers/motor/MotorError.h"

#include "domain/fmt/Logger.h"
#include "../../../../domain/ports/drivers/motor/IMotorDriver.h"
#include "../../../../domain/ports/drivers/valve/IValveDriver.h"

namespace infra::motors {

class G540LptMotorDriver final
    : public domain::ports::IMotorDriver
    , public domain::ports::IValveDriver
{
public:
    G540LptMotorDriver(const MotorDriverPorts& ports,
                       const G540LptMotorDriverConfig& config);
    ~G540LptMotorDriver() override;

    void initialize() override;

    // --- Lifecycle ---
    bool start() override;          // false если LPT не открыт
    void stop() override;
    bool isRunning() const override;
    void abort() override;          // аварийный стоп

    // --- Motion config ---
    void setFrequency(int hz) override;
    int frequency() const override;
    domain::common::MotorFrequencyLimits frequencyLimits() const override;

    void setDirection(domain::common::MotorDirection dir) override;
    domain::common::MotorDirection direction() const override;

    // --- Limits ---
    domain::common::MotorLimitsState limits() const override;

    // --- Observers (motor) ---
    void addObserver(domain::ports::IMotorDriverObserver& o) override;
    void removeObserver(domain::ports::IMotorDriverObserver& o) override;

    // --- Valve driver ---
    bool openInputFlap() override;
    bool openOutputFlap() override;
    bool closeFlaps() override;

    void addObserver(domain::ports::IValveDriverObserver& o) override;
    void removeObserver(domain::ports::IValveDriverObserver& o) override;

    // --- Extra ---
    enum class FlapsState { CloseBoth, OpenInput, OpenOutput };
    void setFlapsState(FlapsState s);


private:
    fmt::Logger logger_;

    struct G540LptImpl;
    std::unique_ptr<G540LptImpl> impl_;

    MotorDriverPorts ports_;
    G540LptMotorDriverConfig config_;

    std::atomic<int> current_hz_;
    std::atomic<domain::common::MotorDirection> current_dir_;
};

} // namespace infra::motors

#endif // CLEANGRADUATOR_G540LPTMOTORDRIVER_H