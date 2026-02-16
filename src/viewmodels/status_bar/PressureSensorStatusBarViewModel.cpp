#include "PressureSensorStatusBarViewModel.h"

#include <chrono>

#include "domain/core/common/PressureSourceError.h"
#include "domain/ports/pressure/IPressureSource.h"

mvvm::PressureSensorStatusBarViewModel::PressureSensorStatusBarViewModel(
    PressureSensorStatusBarViewModelDeps deps
)
    : pressure_source_(deps.pressure_source)
{
    pressure_source_.addObserver(*this);
}

mvvm::PressureSensorStatusBarViewModel::~PressureSensorStatusBarViewModel() {
    pressure_source_.removeObserver(*this);
}

double mvvm::PressureSensorStatusBarViewModel::pressureSpeedPaPerSecond() const {
    return pressure_speed_pa_per_sec_.load(std::memory_order_relaxed);
}

void mvvm::PressureSensorStatusBarViewModel::onPressurePacket(const domain::common::PressurePacket& packet) {
    if (last_packet_.has_value()) {
        const auto dt = packet.timestamp.toDuration() - last_packet_->timestamp.toDuration();
        const auto dt_seconds = std::chrono::duration<double>(dt).count();

        if (dt_seconds > 0.0) {
            const auto delta_pa = packet.pressure.pa() - last_packet_->pressure.pa();
            pressure_speed_pa_per_sec_.store(delta_pa / dt_seconds, std::memory_order_relaxed);
        }
    }

    last_packet_ = packet;
    pressure.set(packet.pressure);
    error.set("");
}

void mvvm::PressureSensorStatusBarViewModel::onPressureSourceOpened() {
    is_opened.set(true);
    error.set("");
}

void mvvm::PressureSensorStatusBarViewModel::onPressureSourceOpenFailed(const domain::common::PressureSourceError& err) {
    is_opened.set(false);
    pressure_speed_pa_per_sec_.store(0.0, std::memory_order_relaxed);
    error.set(err.reason);
}

void mvvm::PressureSensorStatusBarViewModel::onPressureSourceClosed(const domain::common::PressureSourceError& err) {
    is_opened.set(false);
    pressure_speed_pa_per_sec_.store(0.0, std::memory_order_relaxed);
    error.set(err.reason);
}
