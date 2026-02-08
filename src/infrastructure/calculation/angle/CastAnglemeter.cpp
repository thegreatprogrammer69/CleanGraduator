#include "CastAnglemeter.h"
#include "detail/cast_anglemeter.h"

using namespace infra::calculation;

struct CastAnglemeter::CastAnglemeterImpl {
    explicit CastAnglemeterImpl(CastAnglemeterConfig config) {
        anglemeter_config am_config;
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
};

CastAnglemeter::CastAnglemeter(CastAnglemeterPorts ports, CastAnglemeterConfig config)
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
