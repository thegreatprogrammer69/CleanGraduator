#include "FakePressureSource.h"

#include <cmath>
#include <stdexcept>

#include "domain/ports/clock/IClock.h"
#include "domain/core/pressure/PressurePacket.h"
#include "domain/core/pressure/PressureSourceEvent.h"
#include "domain/core/pressure/PressureSourceError.h"
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

    worker_ = std::make_unique<utils::thread::ThreadWorker>(
        [this]() { loop(); }
    );
}

FakePressureSource::~FakePressureSource()
{
    stop();
}

bool FakePressureSource::start()
{
    if (running_)
        return false;

    logger_.info("FakePressureSource starting");

    running_ = true;
    worker_->start();

    notifier_.notifyEvent(
        PressureSourceEvent(PressureSourceEvent::Opened()));

    return true;
}

void FakePressureSource::stop()
{
    if (!running_)
        return;

    running_ = false;
    worker_->pause();

    notifier_.notifyEvent(
        PressureSourceEvent(PressureSourceEvent::Closed()));
}

bool FakePressureSource::isRunning() const noexcept
{
    return running_;
}

void FakePressureSource::addObserver(
        domain::ports::IPressureSourceObserver& observer)
{
    notifier_.addObserver(observer);
}

void FakePressureSource::removeObserver(
        domain::ports::IPressureSourceObserver& observer)
{
    notifier_.removeObserver(observer);
}

void FakePressureSource::addSink(domain::ports::IPressureSink& sink)
{
    notifier_.addSink(sink);
}

void FakePressureSource::removeSink(domain::ports::IPressureSink& sink)
{
    notifier_.removeSink(sink);
}

    void FakePressureSource::loop()
{
    const double fromPa = config_.from.pa();
    const double toPa   = config_.to.pa();
    const double delta  = toPa - fromPa;

    const double durationSec =
        static_cast<double>(config_.duration.count()) / 1000.0;

    while (worker_->isRunning())
    {
        const double nowSec = clock_.now().asSeconds();

        double valuePa = 0.0;

        if (config_.mode == FakePressureSourceConfig::Mode::Ramp)
        {
            const double t = std::fmod(nowSec, durationSec) / durationSec;
            valuePa = fromPa + delta * t;
        }
        else
        {
            const double fullPeriod = durationSec * 2.0;
            const double phase = std::fmod(nowSec, fullPeriod);

            double localT = (phase < durationSec)
                ? phase / durationSec
                : 1.0 - (phase - durationSec) / durationSec;

            valuePa = fromPa + delta * localT;
        }

        PressurePacket packet;
        packet.timestamp = clock_.now();
        packet.pressure =
            Pressure::fromKgfCm2(
                Pressure::fromPa(valuePa).kgfcm2());

        notifier_.notifyPressure(packet);

        platform::sleep(config_.poll_interval);
    }
}

}