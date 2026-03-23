#include "FakeAngleSourceFromVideo.h"

#include <algorithm>
#include <cmath>

#include "domain/core/video/VideoFramePacket.h"
#include "domain/core/angle/AngleSourcePacket.h"
#include "domain/ports/angle/IAngleSink.h"
#include "domain/ports/video/IVideoSource.h"

namespace infra::angle {

namespace {
bool isValidAngle(const domain::common::Angle& angle)
{
    const auto degrees = angle.degrees();
    return std::isfinite(degrees) && degrees >= 0.0 && degrees <= 360.0;
}
} // namespace


FakeAngleSourceFromVideo::FakeAngleSourceFromVideo(
    domain::common::SourceId id,
    AngleSourcePorts ports,
    FakeAngleSourceFromVideoConfig config)
    : id_(id)
    , video_source_(ports.video_source)
    , logger_(ports.logger)
    , config_(config)
{
    video_source_.addSink(*this);
    logger_.info("FakeAngleSourceFromVideo constructed");
}

FakeAngleSourceFromVideo::~FakeAngleSourceFromVideo() {
    stop();
    video_source_.removeSink(*this);
    logger_.info("FakeAngleSourceFromVideo destroyed");
}

bool FakeAngleSourceFromVideo::isRunning() const noexcept {
    return state_.load(std::memory_order_acquire) == State::Started;
}

void FakeAngleSourceFromVideo::start() {
    state_.store(State::Started, std::memory_order_release);
}

void FakeAngleSourceFromVideo::stop() {
    state_.store(State::Stopped, std::memory_order_release);
}

void FakeAngleSourceFromVideo::addSink(domain::ports::IAngleSink& s) {
    sinks_.add(s);
}

void FakeAngleSourceFromVideo::removeSink(domain::ports::IAngleSink& s) {
    sinks_.remove(s);
}

void FakeAngleSourceFromVideo::addObserver(domain::ports::IAngleSourceObserver& o) {
    observers_.add(o);
}

void FakeAngleSourceFromVideo::removeObserver(domain::ports::IAngleSourceObserver& o) {
    observers_.remove(o);
}

double FakeAngleSourceFromVideo::computeAngle(std::uint64_t timestamp_ms) const {
    const double from = config_.from_deg;
    const double to   = config_.to_deg;
    const double span = to - from;

    const std::uint64_t leg = std::max<std::uint64_t>(1, config_.duration_ms);
    const std::uint64_t period = 2 * leg;

    const std::uint64_t t = timestamp_ms % period;

    if (t < leg) {
        // from → to
        const double alpha = static_cast<double>(t) / static_cast<double>(leg);
        return from + alpha * span;
    } else {
        // to → from
        const double alpha = static_cast<double>(t - leg) / static_cast<double>(leg);
        return to - alpha * span;
    }
}

void FakeAngleSourceFromVideo::onVideoFrame(
    const domain::common::VideoFramePacket& packet)
{
    if (state_.load(std::memory_order_acquire) != State::Started) {
        return;
    }

    const double deg = computeAngle(packet.timestamp.asMilliseconds());

    domain::common::Angle angle{};
    angle = domain::common::Angle::fromDegrees(deg);   // предполагается, что у вас поле deg

    logger_.info("Angle: {}", angle);

    if (!isValidAngle(angle)) {
        logger_.warn("filtered invalid angle value: {}", angle);
        return;
    }

    domain::common::AngleSourcePacket out{};
    out.source_id = id_;
    out.timestamp = packet.timestamp;
    out.angle     = angle;

    sinks_.notify([&out](domain::ports::IAngleSink& s) {
        s.onAnglePacket(out);
    });
}

} // namespace infra::angle