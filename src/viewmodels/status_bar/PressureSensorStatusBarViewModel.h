#ifndef CLEANGRADUATOR_PRESSURESENSORSTATUSBARVIEWMODEL_H
#define CLEANGRADUATOR_PRESSURESENSORSTATUSBARVIEWMODEL_H

#include <atomic>
#include <string>

#include "domain/core/common/PressurePacket.h"
#include "domain/core/common/PressureSourceError.h"
#include "domain/core/measurement/Pressure.h"
#include "domain/ports/pressure/IPressureSourceObserver.h"
#include "viewmodels/Observable.h"

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

        double pressureSpeedPaPerSecond() const;

        Observable<bool> is_opened{false};
        Observable<domain::common::Pressure> pressure{};
        Observable<std::string> error{};

    protected:
        void onPressurePacket(const domain::common::PressurePacket& packet) override;
        void onPressureSourceOpened() override;
        void onPressureSourceOpenFailed(const domain::common::PressureSourceError& error) override;
        void onPressureSourceClosed(const domain::common::PressureSourceError& error) override;

    private:
        domain::ports::IPressureSource& pressure_source_;
        std::atomic<double> pressure_speed_pa_per_second_{0.0};

        bool has_previous_packet_{false};
        domain::common::PressurePacket previous_packet_{};
    };

}

#endif //CLEANGRADUATOR_PRESSURESENSORSTATUSBARVIEWMODEL_H
