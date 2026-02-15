#ifndef UNTITLED_WINDM5002PRESSURESENSOR_H
#define UNTITLED_WINDM5002PRESSURESENSOR_H

#include <memory>
#include <domain/ports/pressure/IPressureSource.h>
#include "DM5002RFConfig.h"
#include "PressureSourcePorts.h"
#include "domain/core/measurement/Pressure.h"
#include "domain/core/measurement/Timestamp.h"
#include "domain/fmt/Logger.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"

namespace infra::pressure {

class DM5002RFPressureSensor final : public domain::ports::IPressureSource {
public:
    DM5002RFPressureSensor(const PressureSourcePorts& ports, const DM5002RFConfig& config);
    ~DM5002RFPressureSensor() override;

    bool start() override;
    void stop() override;
    void addObserver(domain::ports::IPressureSourceObserver &observer) override;
    void removeObserver(domain::ports::IPressureSourceObserver &observer) override;

private:
    struct ReadResult {
        bool valid{false};
        domain::common::Timestamp time_point;
        domain::common::Pressure pressure;
    };
    ReadResult readPressure();
    void run();

private:
    struct DM5002RFPressureSensorImpl;
    std::unique_ptr<DM5002RFPressureSensorImpl> impl_;
    const PressureSourcePorts& ports_;
    const DM5002RFConfig& config_;
    fmt::Logger logger_;
    pressure::detail::PressureSourceNotifier notifier_;
};

}

#endif //UNTITLED_WINDM5002PRESSURESENSOR_H