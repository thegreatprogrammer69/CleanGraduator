#include "CastAnglemeter.h"
#include "detail/cast_anglemeter.h"
#include <algorithm>

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
    anglemeter_t* am;
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

domain::common::Angle CastAnglemeter::calculate(const domain::common::AnglemeterInput &input) {
    float angle;
    anglemeterSetImageSize(impl_->am, input.frame->width, input.frame->height);
    anglemeterGetArrowAngle(impl_->am, input.frame->buffer.data, &angle);
    return domain::common::Angle::fromDegrees(angle);
}

void CastAnglemeter::setBrightLim(int brightLim) {
    impl_->am_config.bright_lim = std::clamp(brightLim, 0, 255);
    impl_->updateConfig();
}

void CastAnglemeter::setMaxPairs(int maxPairs) {
    impl_->am_config.max_pairs = std::max(1, maxPairs);
    impl_->updateConfig();
}

void CastAnglemeter::setScanStep(int scanStep) {
    impl_->am_config.scan_step = std::max(1, scanStep);
    impl_->updateConfig();
}

void CastAnglemeter::setOffsets(const std::vector<int>& offsets) {
    impl_->am_config.offsets = offsets;
    impl_->updateConfig();
}
