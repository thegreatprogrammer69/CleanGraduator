#include "CalibrationRecorder.h"

#include "application/ports/video/IVideoAngleSourcesStorage.h"
#include "domain/core/angle/AnglePacket.h"
#include "domain/core/pressure/PressurePacket.h"
#include "domain/ports/angle/IAngleSource.h"
#include "domain/ports/pressure/IPressureSource.h"

using namespace application::orchestrators;

CalibrationRecorder::CalibrationRecorder(CalibrationRecorderPorts ports)
    : logger_(ports.logger)
    , angle_sources_storage_(ports.angle_sources_storage)
    , pressure_source_(ports.pressure_source)
{
    logger_.info("constructor called");
}

CalibrationRecorder::~CalibrationRecorder() {
    logger_.info("destructor called. stopping..");
    stop();
}

bool CalibrationRecorder::start(ports::CalibrationRecorderInput input) {
    if (is_recording_.exchange(true, std::memory_order_acq_rel)) {
        logger_.warn("already recording");
        return false;
    }

    if (!pressure_source_.isRunning()) {
        logger_.warn("pressure source not running");
        is_recording_.store(false, std::memory_order_release);
        return false;
    }

    logger_.info("starting session");

    // Сохраняем вход
    active_sources_ids_ = std::move(input.active_sources_ids);
    pressure_unit_      = input.pressure_unit;
    angle_unit_         = input.angle_unit;

    dataset_.clear();
    unsubscribers_.clear();

    subscribe();

    return true;
}

void CalibrationRecorder::stop() {
    if (!is_recording_.exchange(false, std::memory_order_acq_rel)) {
        logger_.info("attempt to stop. is already stopped");
        return;
    }

    logger_.info("stopping session");

    unsubscribe();
}

void CalibrationRecorder::resetSession() {
    dataset_.clear();
}

bool CalibrationRecorder::isRecording() const {
    return is_recording_.load(std::memory_order_relaxed);
}

void CalibrationRecorder::onAngle(const domain::common::AnglePacket &a) noexcept {
    dataset_.pushAgle(a.source_id, a.timestamp.asSeconds(), a.angle.to(angle_unit_));
}

void CalibrationRecorder::onPressurePacket(const domain::common::PressurePacket &p) {
    dataset_.pushPressure(p.timestamp.asSeconds(), p.pressure.to(pressure_unit_));
}

void CalibrationRecorder::subscribe()
{
    logger_.info("subscribing to sources");

    for (const auto& id : active_sources_ids_) {

        auto angle_source = angle_sources_storage_.at(id);
        if (!angle_source)
            continue;

        angle_source->angle_source.addSink(*this);

        // сохраняем отписку
        unsubscribers_.emplace_back(
            [this, id]() {
                auto src = angle_sources_storage_.at(id);
                if (src) {
                    src->angle_source.removeSink(*this);
                }
            }
        );
    }

    pressure_source_.addObserver(*this);

    unsubscribers_.emplace_back(
        [this]() {
            pressure_source_.removeObserver(*this);
        }
    );
}

void CalibrationRecorder::unsubscribe()
{
    logger_.info("unsubscribing");

    for (auto& fn : unsubscribers_) {
        try {
            fn();
        } catch (...) {
            logger_.error("unsubscribe callback threw");
        }
    }

    unsubscribers_.clear();
}
