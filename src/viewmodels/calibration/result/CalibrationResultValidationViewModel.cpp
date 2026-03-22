#include "CalibrationResultValidationViewModel.h"

#include <cmath>

#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "viewmodels/control/CalibrationSessionControlViewModel.h"
#include "viewmodels/settings/InfoSettingsViewModel.h"

using namespace mvvm;
using namespace domain::common;

namespace {
constexpr float kSweepAngleUpperLimit = 290.0f;
constexpr float kSweepAngleLowerLimit = 260.0f;
constexpr float kSweepAngleLowerLimitKu = 245.0f;

CalibrationCellKey makeKey(const PointId& point, const SourceId& source, MotorDirection direction)
{
    return CalibrationCellKey{point, source, direction};
}

std::optional<float> angleOf(const CalibrationResult& result, const CalibrationCellKey& key)
{
    const auto cell = result.cell(key);
    if (!cell || !cell->angle()) {
        return std::nullopt;
    }
    return cell->angle();
}
}

CalibrationResultValidationViewModel::CalibrationResultValidationViewModel(CalibrationResultValidationViewModelDeps deps)
    : result_source_(deps.result_source)
    , context_provider_(deps.context_provider)
    , info_settings_(deps.info_settings)
    , session_control_(deps.session_control)
{
    result_source_.addObserver(*this);
    current_result_ = result_source_.currentResult();

    precision_sub_ = info_settings_.selectedPrecision.subscribe([this](const auto&) {
        revalidate();
    }, false);
    ku_sub_ = session_control_.correction_u.subscribe([this](const auto&) {
        revalidate();
    }, false);

    revalidate();
}

CalibrationResultValidationViewModel::~CalibrationResultValidationViewModel()
{
    result_source_.removeObserver(*this);
}

void CalibrationResultValidationViewModel::onCalibrationResultUpdated(const CalibrationResult& result)
{
    current_result_ = result;
    revalidate();
}

const std::optional<ClibrationResultValidation>& CalibrationResultValidationViewModel::currentValidation() const
{
    return current_validation_;
}

void CalibrationResultValidationViewModel::addObserver(domain::ports::IClibrationResultValidationObserver& observer)
{
    observers_.add(observer);
}

void CalibrationResultValidationViewModel::removeObserver(domain::ports::IClibrationResultValidationObserver& observer)
{
    observers_.remove(observer);
}

void CalibrationResultValidationViewModel::revalidate()
{
    if (!current_result_) {
        current_validation_ = ClibrationResultValidation{};
        publish();
        return;
    }

    current_validation_ = buildValidation(*current_result_);
    publish();
}

ClibrationResultValidation CalibrationResultValidationViewModel::buildValidation(const CalibrationResult& result) const
{
    ClibrationResultValidation validation;
    validateSweepAngle(result, validation);
    validateHysteresis(result, validation);
    return validation;
}

void CalibrationResultValidationViewModel::validateSweepAngle(const CalibrationResult& result,
                                                             ClibrationResultValidation& validation) const
{
    if (result.points().size() < 2) {
        return;
    }

    const auto& firstPoint = result.points().front();
    const auto& lastPoint = result.points().back();
    const float lowerBound = session_control_.correction_u.get_copy() ? kSweepAngleLowerLimitKu : kSweepAngleLowerLimit;

    for (const auto& source : result.sources()) {
        const auto firstAngle = angleOf(result, makeKey(firstPoint, source, MotorDirection::Forward));
        const auto lastAngle = angleOf(result, makeKey(lastPoint, source, MotorDirection::Forward));
        if (!firstAngle || !lastAngle) {
            continue;
        }

        const float delta = *lastAngle - *firstAngle;
        if (delta >= lowerBound && delta <= kSweepAngleUpperLimit) {
            continue;
        }

        const auto message = std::string("Размах прямого хода вне допустимого диапазона: ") + std::to_string(delta);
        validation.setIssues(makeKey(firstPoint, source, MotorDirection::Forward), {{CalibrationIssueSeverity::Warning, message}});
        validation.setIssues(makeKey(lastPoint, source, MotorDirection::Forward), {{CalibrationIssueSeverity::Warning, message}});
    }
}

void CalibrationResultValidationViewModel::validateHysteresis(const CalibrationResult& result,
                                                             ClibrationResultValidation& validation) const
{
    const float limit = hysteresisLimit(result);
    if (limit <= 0.0f) {
        return;
    }

    for (const auto& point : result.points()) {
        for (const auto& source : result.sources()) {
            const auto forwardAngle = angleOf(result, makeKey(point, source, MotorDirection::Forward));
            const auto backwardAngle = angleOf(result, makeKey(point, source, MotorDirection::Backward));
            if (!forwardAngle || !backwardAngle) {
                continue;
            }

            const float delta = std::fabs(*forwardAngle - *backwardAngle);
            if (delta <= limit) {
                continue;
            }

            const auto message = std::string("Превышение гистерезиса: ") + std::to_string(delta) +
                " > " + std::to_string(limit);
            validation.setIssues(makeKey(point, source, MotorDirection::Forward), {{CalibrationIssueSeverity::Error, message}});
            validation.setIssues(makeKey(point, source, MotorDirection::Backward), {{CalibrationIssueSeverity::Error, message}});
        }
    }
}

float CalibrationResultValidationViewModel::hysteresisLimit(const CalibrationResult& result) const
{
    const auto context = context_provider_.current();
    if (!context) {
        return 0.0f;
    }

    if (result.points().size() < 2) {
        return 0.0f;
    }

    const auto& firstPoint = result.points().front();
    const auto& lastPoint = result.points().back();
    float maxSweep = 0.0f;

    for (const auto& source : result.sources()) {
        const auto firstAngle = angleOf(result, makeKey(firstPoint, source, MotorDirection::Forward));
        const auto lastAngle = angleOf(result, makeKey(lastPoint, source, MotorDirection::Forward));
        if (!firstAngle || !lastAngle) {
            continue;
        }
        maxSweep = std::max(maxSweep, std::fabs(*lastAngle - *firstAngle));
    }

    if (maxSweep <= 0.0f) {
        return 0.0f;
    }

    return maxSweep * static_cast<float>(context->precision.precision.value / 100.0);
}

void CalibrationResultValidationViewModel::publish()
{
    if (!current_validation_) {
        current_validation_ = ClibrationResultValidation{};
    }

    observers_.notify([this](domain::ports::IClibrationResultValidationObserver& observer) {
        observer.onClibrationResultValidationUpdated(*current_validation_);
    });
}
