#include "CalibrationResultValidationSource.h"

#include <cmath>
#include <limits>
#include <optional>
#include <string>

using namespace application::orchestrators;
using namespace domain::common;

namespace {
float angleFor(const CalibrationResult& result, const CalibrationCellKey& key)
{
    const auto cell = result.cell(key);
    if (!cell || !cell->angle()) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return *cell->angle();
}

bool isValid(float value)
{
    return !std::isnan(value);
}
}

CalibrationResultValidationSource::CalibrationResultValidationSource(
    CalibrationResultValidationSourceDeps deps)
    : logger_(deps.logger)
    , deps_(deps)
{
    deps_.result_source.addObserver(*this);
    current_result_ = deps_.result_source.currentResult();
    rebuild();
}

CalibrationResultValidationSource::~CalibrationResultValidationSource()
{
    deps_.result_source.removeObserver(*this);
}

void CalibrationResultValidationSource::onCalibrationResultUpdated(const CalibrationResult& result)
{
    current_result_ = result;
    rebuild();
}

const std::optional<CalibrationResultValidation>& CalibrationResultValidationSource::currentValidation() const
{
    return current_validation_;
}

void CalibrationResultValidationSource::addObserver(domain::ports::ICalibrationResultValidationObserver& observer)
{
    observers_.add(observer);
}

void CalibrationResultValidationSource::removeObserver(domain::ports::ICalibrationResultValidationObserver& observer)
{
    observers_.remove(observer);
}

void CalibrationResultValidationSource::requestRefresh()
{
    rebuild();
}

void CalibrationResultValidationSource::rebuild()
{
    if (!current_result_) {
        current_validation_ = CalibrationResultValidation{};
        notifyObservers();
        return;
    }

    CalibrationResultValidation validation;
    const auto& result = *current_result_;
    const float min_span = isKuEnabled() ? 245.0F : 260.0F;
    constexpr float max_span = 280.0F;
    const float precision_percent = resolvePrecisionPercent();

    if (!result.points().empty()) {
        const auto& first_point = result.points().front();
        const auto& last_point = result.points().back();

        for (const auto& source : result.sources()) {
            CalibrationCellKey first_key{first_point, source, MotorDirection::Forward};
            CalibrationCellKey last_key{last_point, source, MotorDirection::Forward};
            const float first_angle = angleFor(result, first_key);
            const float last_angle = angleFor(result, last_key);

            if (isValid(first_angle) && isValid(last_angle)) {
                const float span = last_angle - first_angle;
                if (span > max_span) {
                    for (const auto& point : result.points()) {
                        validation.addIssue(
                            CalibrationCellKey{point, source, MotorDirection::Forward},
                            CalibrationResultValidationIssue{
                                CalibrationIssueSeverity::Error,
                                CalibrationValidationIssueKind::AngleSpanTooHigh,
                                "Угол шкалы прямого хода больше 280°"
                            });
                    }
                } else if (span < min_span) {
                    const std::string message = min_span < 260.0F
                        ? "Угол шкалы прямого хода меньше 245°"
                        : "Угол шкалы прямого хода меньше 260°";
                    for (const auto& point : result.points()) {
                        validation.addIssue(
                            CalibrationCellKey{point, source, MotorDirection::Forward},
                            CalibrationResultValidationIssue{
                                CalibrationIssueSeverity::Error,
                                CalibrationValidationIssueKind::AngleSpanTooLow,
                                message
                            });
                    }
                }

                const float hysteresis_limit = std::abs(span) * (precision_percent / 100.0F);
                for (const auto& point : result.points()) {
                    const CalibrationCellKey forward_key{point, source, MotorDirection::Forward};
                    const CalibrationCellKey backward_key{point, source, MotorDirection::Backward};
                    const float forward_angle = angleFor(result, forward_key);
                    const float backward_angle = angleFor(result, backward_key);

                    if (!isValid(forward_angle) || !isValid(backward_angle)) {
                        continue;
                    }

                    if (std::abs(forward_angle - backward_angle) > hysteresis_limit) {
                        const auto issue = CalibrationResultValidationIssue{
                            CalibrationIssueSeverity::Warning,
                            CalibrationValidationIssueKind::HysteresisExceeded,
                            "Превышение гистерезиса для выбранного класса точности"
                        };
                        validation.addIssue(forward_key, issue);
                        validation.addIssue(backward_key, issue);
                    }
                }
            }
        }
    }

    current_validation_ = validation;
    notifyObservers();
}

void CalibrationResultValidationSource::notifyObservers() const
{
    if (!current_validation_) {
        return;
    }

    observers_.notify([this](domain::ports::ICalibrationResultValidationObserver& observer) {
        observer.onCalibrationResultValidationUpdated(*current_validation_);
    });
}

float CalibrationResultValidationSource::resolvePrecisionPercent() const
{
    const auto settings = deps_.settings_storage.loadInfoSettings();
    const auto precision = deps_.precision_catalog.at(settings.precision_idx);
    return precision ? precision->precision.value : 1.0F;
}

bool CalibrationResultValidationSource::isKuEnabled() const
{
    return deps_.settings_storage.loadInfoSettings().ku_enabled;
}
