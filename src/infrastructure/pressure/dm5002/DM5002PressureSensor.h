#ifndef CLEANGRADUATOR_DM5002PRESSURESENSOR_H
#define CLEANGRADUATOR_DM5002PRESSURESENSOR_H

#include <memory>

#include <domain/ports/pressure/IPressureSource.h>

#include "DM5002PressureSensorConfig.h"
#include "../PressureSourcePorts.h"

#include "domain/core/measurement/Pressure.h"
#include "domain/core/measurement/Timestamp.h"
#include "domain/fmt/Logger.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"

namespace domain::ports { class IClock; }

namespace infra::pressure {

    class DM5002PressureSensor final : public domain::ports::IPressureSource {
    public:
        DM5002PressureSensor(PressureSourcePorts ports, DM5002PressureSensorConfig config);
        ~DM5002PressureSensor() override;

        bool start() override;
        void stop() override;
        bool isRunning() const noexcept override;

        void addObserver(domain::ports::IPressureSourceObserver& observer) override;
        void removeObserver(domain::ports::IPressureSourceObserver& observer) override;

    private:
        struct ReadResult {
            bool valid{false};
            domain::common::Timestamp time_point{};
            domain::common::Pressure pressure{};
        };

        ReadResult readPressure();
        void run();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;

        DM5002PressureSensorConfig config_;
        fmt::Logger logger_;
        domain::ports::IClock& clock_;
        detail::PressureSourceNotifier notifier_;
    };

} // namespace infra::pressure

#endif // CLEANGRADUATOR_DM5002PRESSURESENSOR_H