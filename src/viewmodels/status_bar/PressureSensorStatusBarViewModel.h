#ifndef CLEANGRADUATOR_PRESSURESENSORSTATUSBARVIEWMODEL_H
#define CLEANGRADUATOR_PRESSURESENSORSTATUSBARVIEWMODEL_H

#include <atomic>
#include <optional>
#include <string>

#include "domain/core/common/PressurePacket.h"
#include "domain/core/measurement/Pressure.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "viewmodels/Observable.h"

namespace application::models {
    struct PressureUnit;
}

namespace domain::ports {
    struct IPressureSource;
}

namespace mvvm {

    struct PressureSensorStatusBarViewModelDeps {
        domain::ports::IPressureSource& pressure_source;
    };

    class PressureSensorStatusBarViewModel final : public domain::ports::IPressureSourceObserver {
    public:
        explicit PressureSensorStatusBarViewModel(PressureSensorStatusBarViewModelDeps deps);
        ~PressureSensorStatusBarViewModel() override;

        domain::common::Pressure pressureSpeedPerSecond() const;

        Observable<bool> is_opened{false};
        Observable<domain::common::Pressure> pressure{};
        Observable<std::string> error{};

    protected:
        void onPressurePacket(const domain::common::PressurePacket& packet) override;
        void onPressureSourceOpened() override;
        void onPressureSourceOpenFailed(const domain::common::PressureSourceError& err) override;
        void onPressureSourceClosed(const domain::common::PressureSourceError& err) override;

    private:
        domain::ports::IPressureSource& pressure_source_;

        std::atomic<double> pressure_speed_pa_per_sec_{0.0};
        std::atomic<domain::common::PressureUnit> pressure_unit_ = domain::common::PressureUnit::Pa;

        std::optional<domain::common::Pressure> last_pressure_{};
        std::optional<std::chrono::steady_clock::time_point> last_time_{};

    };

} // namespace mvvm

#endif //CLEANGRADUATOR_PRESSURESENSORSTATUSBARVIEWMODEL_H
