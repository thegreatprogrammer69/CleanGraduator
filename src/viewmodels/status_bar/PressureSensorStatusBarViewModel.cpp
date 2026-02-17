#include "PressureSensorStatusBarViewModel.h"

#include <chrono>

#include "domain/ports/pressure/IPressureSource.h"

mvvm::PressureSensorStatusBarViewModel::PressureSensorStatusBarViewModel(PressureSensorStatusBarViewModelDeps deps)
    : pressure_source_(deps.pressure_source)
{
    pressure_source_.addObserver(*this);
}

mvvm::PressureSensorStatusBarViewModel::~PressureSensorStatusBarViewModel() {
    pressure_source_.removeObserver(*this);
}

double mvvm::PressureSensorStatusBarViewModel::pressureSpeedPaPerSecond() const {
    return pressure_speed_pa_per_second_.load(std::memory_order_relaxed);
}

void mvvm::PressureSensorStatusBarViewModel::onPressurePacket(const domain::common::PressurePacket& packet) {
    if (has_previous_packet_) {
        const auto dt = packet.timestamp.toDuration() - previous_packet_.timestamp.toDuration();
        const auto dt_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();

        if (dt_seconds > 0.0) {
            const auto dp_pa = packet.pressure.pa() - previous_packet_.pressure.pa();
            pressure_speed_pa_per_second_.store(dp_pa / dt_seconds, std::memory_order_relaxed);
        }
    }

    previous_packet_ = packet;
    has_previous_packet_ = true;

    pressure.set(packet.pressure);
}

void mvvm::PressureSensorStatusBarViewModel::onPressureSourceOpened() {
    is_opened.set(true);
    error.set("");
}

void mvvm::PressureSensorStatusBarViewModel::onPressureSourceOpenFailed(const domain::common::PressureSourceError& error) {
    is_opened.set(false);
    this->error.set(error.reason);
    pressure_speed_pa_per_second_.store(0.0, std::memory_order_relaxed);
}

void mvvm::PressureSensorStatusBarViewModel::onPressureSourceClosed(const domain::common::PressureSourceError& error) {
    is_opened.set(false);
    this->error.set(error.reason);
    pressure_speed_pa_per_second_.store(0.0, std::memory_order_relaxed);
}
