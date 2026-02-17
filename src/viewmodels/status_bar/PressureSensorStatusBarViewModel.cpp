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

bool mvvm::PressureSensorStatusBarViewModel::open() {
    return pressure_source_.start();
}

void mvvm::PressureSensorStatusBarViewModel::close() {
    pressure_source_.stop();
}

domain::common::Pressure mvvm::PressureSensorStatusBarViewModel::pressureSpeedPerSecond() const {
    auto p = domain::common::Pressure::fromPa(
        pressure_speed_pa_per_sec_.load(std::memory_order_relaxed)
    );
    p.setUnit(pressure_unit_.load(std::memory_order_relaxed));
    return p;
}

void mvvm::PressureSensorStatusBarViewModel::onPressurePacket(
    const domain::common::PressurePacket& packet)
{
    const auto now = std::chrono::steady_clock::now();

    if (last_pressure_.has_value() && last_time_.has_value()) {

        const auto dt = now - *last_time_;
        const double dt_seconds = std::chrono::duration<double>(dt).count();

        if (dt_seconds > 0.0) {
            const double delta_pa =
                packet.pressure.pa() - last_pressure_->pa();

            pressure_speed_pa_per_sec_.store(
                delta_pa / dt_seconds,
                std::memory_order_relaxed
            );

            pressure_unit_.store(packet.pressure.unit(), std::memory_order_relaxed);

        }
    }

    last_pressure_ = packet.pressure;
    last_time_ = now;

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
    last_pressure_.reset();
    last_time_.reset();
    error.set(err.reason);
}
