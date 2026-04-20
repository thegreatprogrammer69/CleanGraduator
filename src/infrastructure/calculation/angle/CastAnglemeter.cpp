#include "CastAnglemeter.h"
#include "detail/cast_anglemeter.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

using namespace infra::calc;

struct CastAnglemeter::CastAnglemeterImpl {
    explicit CastAnglemeterImpl(CastAnglemeterConfig config) {
        am_config.bright_lim = config.bright_lim;
        am_config.max_pairs = config.max_pairs;
        am_config.offsets = config.offsets;
        am_config.scan_step = config.scan_step;
        anglemeterCreate(&am, am_config);
    }
    ~CastAnglemeterImpl() {
        anglemeterDestroy(am);
    }
    anglemeter_t* am = nullptr;
    anglemeter_config am_config;
    void updateConfig() {
        anglemeterSetConfig(am, am_config);
    }
};

CastAnglemeter::CastAnglemeter(AnglemeterPorts ports, CastAnglemeterConfig config)
    : impl_(std::make_unique<CastAnglemeterImpl>(config))
    , logger_(ports.logger)
{
}

CastAnglemeter::~CastAnglemeter() {

}

domain::common::Angle CastAnglemeter::calculate(const domain::common::AngleCalculatorInput &input) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (input.frame == nullptr) {
        throw std::invalid_argument("Frame is null.");
    }

    if (input.frame->format != domain::common::PixelFormat::RGB24) {
        throw std::logic_error("Unsupported pixel format. Expected RGB24.");
    }

    if (input.frame->width <= 0 || input.frame->height <= 0) {
        throw std::invalid_argument("Invalid frame dimensions.");
    }

    const size_t width = static_cast<size_t>(input.frame->width);
    const size_t height = static_cast<size_t>(input.frame->height);
    if (width > (std::numeric_limits<size_t>::max() / height / 3u)) {
        throw std::overflow_error("Frame size overflows RGB24 buffer calculation.");
    }

    const size_t expected_size = width * height * 3u;
    if (input.frame->buffer.data == nullptr || input.frame->buffer.size < expected_size) {
        throw std::invalid_argument("RGB24 frame buffer is null or too small.");
    }

    if (impl_->am == nullptr) {
        throw std::runtime_error("Angle meter is not initialized.");
    }

    float angle = 0.0f;
    anglemeterSetImageSize(impl_->am, input.frame->width, input.frame->height);
    if (!anglemeterGetArrowAngle(impl_->am, input.frame->buffer.data, &angle)) {
        return domain::common::Angle::fromDegrees(-1000);
    }

    angle = 180 - angle;
    if (angle <= 0.0f) angle += 360.0f;
    return domain::common::Angle::fromDegrees(angle);
}

void CastAnglemeter::setBrightLim(int brightLim) {
    std::lock_guard<std::mutex> lock(mutex_);
    impl_->am_config.bright_lim = std::clamp(brightLim, 0, 255);
    impl_->updateConfig();
}

void CastAnglemeter::setMaxPairs(int maxPairs) {
    std::lock_guard<std::mutex> lock(mutex_);
    impl_->am_config.max_pairs = std::max(1, maxPairs);
    impl_->updateConfig();
}

void CastAnglemeter::setScanStep(int scanStep) {
    std::lock_guard<std::mutex> lock(mutex_);
    impl_->am_config.scan_step = std::max(1, scanStep);
    impl_->updateConfig();
}

void CastAnglemeter::setOffsets(const std::vector<int>& offsets) {
    std::lock_guard<std::mutex> lock(mutex_);
    impl_->am_config.offsets = offsets;
    impl_->updateConfig();
}
