#include "ClibrationResultValidationSource.h"

#include <cmath>

using namespace application::orchestrators;
using namespace domain::common;

namespace {

float hysteresisLimit(float span, double precisionClass)
{
    return span * static_cast<float>(precisionClass / 100.0);
}

} // namespace

ClibrationResultValidationSource::ClibrationResultValidationSource(ClibrationResultValidationSourceDeps deps)
    : result_source_(deps.result_source)
    , context_provider_(deps.context_provider)
{
    result_source_.addObserver(*this);
    if (const auto& result = result_source_.currentResult(); result.has_value()) {
        onCalibrationResultUpdated(*result);
    }
}

ClibrationResultValidationSource::~ClibrationResultValidationSource()
{
    result_source_.removeObserver(*this);
}

const std::optional<ClibrationResultValidation>& ClibrationResultValidationSource::currentValidation() const
{
    return current_validation_;
}

void ClibrationResultValidationSource::addObserver(domain::ports::IClibrationResultObserver& observer)
{
    observers_.add(observer);
}

void ClibrationResultValidationSource::removeObserver(domain::ports::IClibrationResultObserver& observer)
{
    observers_.remove(observer);
}

void ClibrationResultValidationSource::revalidate()
{
    if (!current_result_) {
        current_validation_.reset();
        return;
    }

    current_validation_ = buildValidation(*current_result_);
    notifyObservers();
}

void ClibrationResultValidationSource::setKuModeEnabled(bool enabled)
{
    if (ku_mode_enabled_ == enabled) {
        return;
    }

    ku_mode_enabled_ = enabled;
    revalidate();
}

bool ClibrationResultValidationSource::kuModeEnabled() const
{
    return ku_mode_enabled_;
}

void ClibrationResultValidationSource::onCalibrationResultUpdated(const CalibrationResult& result)
{
    current_result_ = result;
    current_validation_ = buildValidation(result);
    notifyObservers();
}

ClibrationResultValidation ClibrationResultValidationSource::buildValidation(const CalibrationResult& result) const
{
    ClibrationResultValidation validation;

    const auto calibration_context = context_provider_.current();
    const double precision = calibration_context ? calibration_context->precision.precision.value : 0.0;

    for (const auto& source : result.sources()) {
        const auto first_point = result.points().front();
        const auto last_point = result.points().back();

        CalibrationCellKey first_key{first_point, source, MotorDirection::Forward};
        CalibrationCellKey last_key{last_point, source, MotorDirection::Forward};

        const auto first_cell = result.cell(first_key);
        const auto last_cell = result.cell(last_key);

        if (first_cell && first_cell->angle() && last_cell && last_cell->angle()) {
            const float span = *last_cell->angle() - *first_cell->angle();
            const float min_span = ku_mode_enabled_ ? 245.0f : 260.0f;
            if (span > 290.0f || span < min_span) {
                const auto message = std::string("Выход за допустимый диапазон угла прямого хода: ") + std::to_string(span);
                validation.setIssues(first_key, {{CalibrationIssueSeverity::Warning, message}});
                validation.setIssues(last_key, {{CalibrationIssueSeverity::Warning, message}});
            }

            const float max_hysteresis = hysteresisLimit(std::fabs(span), precision);
            if (max_hysteresis > 0.0f) {
                for (const auto& point : result.points()) {
                    CalibrationCellKey forward_key{point, source, MotorDirection::Forward};
                    CalibrationCellKey backward_key{point, source, MotorDirection::Backward};
                    const auto forward_cell = result.cell(forward_key);
                    const auto backward_cell = result.cell(backward_key);

                    if (!forward_cell || !backward_cell || !forward_cell->angle() || !backward_cell->angle()) {
                        continue;
                    }

                    const float hysteresis = std::fabs(*forward_cell->angle() - *backward_cell->angle());
                    if (hysteresis > max_hysteresis) {
                        const auto message = std::string("Превышение гистерезиса: ") + std::to_string(hysteresis)
                            + ", допустимо: " + std::to_string(max_hysteresis);
                        validation.setIssues(forward_key, {{CalibrationIssueSeverity::Error, message}});
                        validation.setIssues(backward_key, {{CalibrationIssueSeverity::Error, message}});
                    }
                }
            }
        }
    }

    return validation;
}

void ClibrationResultValidationSource::notifyObservers() const
{
    if (!current_result_ || !current_validation_) {
        return;
    }

    observers_.notify([this](domain::ports::IClibrationResultObserver& observer) {
        observer.onClibrationResultUpdated(*current_result_, *current_validation_);
    });
}
