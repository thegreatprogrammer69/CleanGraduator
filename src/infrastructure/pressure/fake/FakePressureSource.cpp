#include "FakePressureSource.h"

#include <cmath>

#include "domain/ports/clock/IClock.h"
#include "domain/core/pressure/PressurePacket.h"
#include "domain/core/pressure/PressureSourceError.h"
#include "domain/core/pressure/PressureSourceEvent.h"
#include "infrastructure/platform/sleep/sleep.h"

namespace infra::pressure {
    using namespace domain::common;

FakePressureSource::FakePressureSource(
        PressureSourcePorts ports,
        FakePressureSourceConfig config)
    : config_(std::move(config))
    , logger_(ports.logger)
    , clock_(ports.clock)
{
    if (config_.duration.count() <= 0)
        throw std::invalid_argument("FakePressureSource: duration must be > 0");

    if (config_.poll_interval.count() <= 0)
        throw std::invalid_argument("FakePressureSource: poll_interval must be > 0");

    logger_.info("FakePressureSource constructed");
}

FakePressureSource::~FakePressureSource() {
    stop();
}

bool FakePressureSource::start() {
    std::lock_guard<std::mutex> lk(lifecycle_mtx_);

    if (running_.load(std::memory_order_acquire))
        return false;

    if (worker_.joinable())
        worker_.join();

    stop_requested_.store(false);
    running_.store(true);

    worker_ = std::thread(&FakePressureSource::run, this);
    return true;
}

void FakePressureSource::stop() {
    std::lock_guard<std::mutex> lk(lifecycle_mtx_);

    if (!running_.load() && !worker_.joinable())
        return;

    stop_requested_.store(true);

    if (worker_.joinable())
        worker_.join();

    running_.store(false);
}

bool FakePressureSource::isRunning() const noexcept {
    return running_.load() && !stop_requested_.load();
}

void FakePressureSource::addObserver(
        domain::ports::IPressureSourceObserver& observer) {
    notifier_.addObserver(observer);
}

void FakePressureSource::removeObserver(
        domain::ports::IPressureSourceObserver& observer) {
    notifier_.removeObserver(observer);
}

void FakePressureSource::run() {

    notifier_.notifyEvent(PressureSourceEvent(PressureSourceEvent::Opened()));

    const double fromPa = config_.from.pa();
    const double toPa   = config_.to.pa();
    const double delta  = toPa - fromPa;

    const double durationSec =
        static_cast<double>(config_.duration.count()) / 1000.0;

    while (!stop_requested_.load()) {

        const double nowSec = clock_.now().asSeconds();

        double t = std::fmod(nowSec, durationSec) / durationSec;

        double valuePa;

        if (config_.mode == FakePressureSourceConfig::Mode::Ramp) {
            valuePa = fromPa + delta * t;
        }
        else { // PingPong
            // период 2*duration
            const double fullPeriod = durationSec * 2.0;
            const double phase = std::fmod(nowSec, fullPeriod);

            double localT;
            if (phase < durationSec) {
                localT = phase / durationSec;
            } else {
                localT = 1.0 - (phase - durationSec) / durationSec;
            }

            valuePa = fromPa + delta * localT;
        }

        domain::common::PressurePacket packet;
        packet.timestamp = clock_.now();
        packet.pressure  = domain::common::Pressure::fromKgfCm2(domain::common::Pressure::fromPa(valuePa).kgfcm2());

        notifier_.notifyPressure(packet);

        platform::sleep(config_.poll_interval);
    }

    notifier_.notifyEvent(PressureSourceEvent(PressureSourceEvent::Closed()));
    running_.store(false);
}

}