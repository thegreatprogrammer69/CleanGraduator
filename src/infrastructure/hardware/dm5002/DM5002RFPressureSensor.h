#ifndef UNTITLED_WINDM5002PRESSURESENSOR_H
#define UNTITLED_WINDM5002PRESSURESENSOR_H

#include <memory>
#include <domain/ports/inbound/IPressureSource.h>
#include "DM5002RFConfig.h"
#include "PressureSensorPorts.h"
#include "domain/core/measurement/Timestamp.h"
#include "domain/fmt/FmtLogger.h"

namespace infra::hardware {

class DM5002RFPressureSensor final : public domain::ports::IPressureSource {
public:
    DM5002RFPressureSensor(const PressureSensorPorts& ports, const DM5002RFConfig& config);
    ~DM5002RFPressureSensor() override;

    bool start() override;
    void stop() override;
    void addSink(domain::ports::IPressureSink &sink) override;
    void removeSink(domain::ports::IPressureSink &sink) override;

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
    const PressureSensorPorts& ports_;
    const DM5002RFConfig& config_;
    fmt::FmtLogger logger_;
};

}

#endif //UNTITLED_WINDM5002PRESSURESENSOR_H