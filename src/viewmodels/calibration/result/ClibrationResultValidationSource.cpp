#include "ClibrationResultValidationSource.h"

#include <cmath>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "viewmodels/control/CalibrationSessionControlViewModel.h"
#include "viewmodels/settings/InfoSettingsViewModel.h"

using namespace mvvm;
using namespace domain::common;

namespace {

void addSpanIssue(ClibrationResultValidation& validation,
                  const CalibrationCellKey& key,
                  float span,
                  float min_span,
                  float max_span)
{
    validation.addIssue(key, {
        ClibrationResultValidationIssue::Type::ForwardAngleSpanOutOfRange,
        "Диапазон прямого хода вне нормы: " + std::to_string(span) +
            "°, ожидается от " + std::to_string(min_span) + "° до " + std::to_string(max_span) + "°"
    });
}

void addHysteresisIssue(ClibrationResultValidation& validation,
                        const CalibrationCellKey& key,
                        float diff,
                        double limit)
{
    validation.addIssue(key, {
        ClibrationResultValidationIssue::Type::HysteresisExceeded,
        "Превышен гистерезис: " + std::to_string(diff) +
            "°, допустимо не более " + std::to_string(limit) + "°"
    });
}

}

ClibrationResultValidationSource::ClibrationResultValidationSource(ClibrationResultValidationSourceDeps deps)
    : deps_(deps)
{
    deps_.result_source.addObserver(*this);
    current_result_ = deps_.result_source.currentResult();

    precision_sub_ = deps_.info_settings.selectedPrecision.subscribe([this](const auto&) {
        revalidate();
    }, false);

    ku_sub_ = deps_.calibration_control.is_ku_enabled.subscribe([this](const auto&) {
        revalidate();
    }, false);

    revalidate();
}

ClibrationResultValidationSource::~ClibrationResultValidationSource() {
    deps_.result_source.removeObserver(*this);
}

void ClibrationResultValidationSource::onCalibrationResultUpdated(const CalibrationResult& result) {
    current_result_ = result;
    revalidate();
}

const std::optional<ClibrationResultValidation>& ClibrationResultValidationSource::currentValidation() const {
    return current_validation_;
}

void ClibrationResultValidationSource::addObserver(domain::ports::IClibrationResultObserver& observer) {
    observers_.add(observer);
}

void ClibrationResultValidationSource::removeObserver(domain::ports::IClibrationResultObserver& observer) {
    observers_.remove(observer);
}

void ClibrationResultValidationSource::revalidate() {
    if (!current_result_) {
        current_validation_.reset();
        return;
    }

    ClibrationResultValidation validation;
    const auto& result = *current_result_;
    const float min_span = isKuEnabled() ? 245.0f : 260.0f;
    const float max_span = 290.0f;

    if (!result.points().empty()) {
        const auto first_point = result.points().front();
        const auto last_point = result.points().back();

        for (const auto& source : result.sources()) {
            CalibrationCellKey first_key{first_point, source, MotorDirection::Forward};
            CalibrationCellKey last_key{last_point, source, MotorDirection::Forward};
            const auto first_cell = result.cell(first_key);
            const auto last_cell = result.cell(last_key);

            if (first_cell && last_cell && first_cell->angle() && last_cell->angle()) {
                const float span = *last_cell->angle() - *first_cell->angle();
                if (span < min_span || span > max_span) {
                    addSpanIssue(validation, first_key, span, min_span, max_span);
                    addSpanIssue(validation, last_key, span, min_span, max_span);
                }
            }
        }
    }

    const double precision_percent = hysteresisLimitPercent();
    for (const auto& point : result.points()) {
        for (const auto& source : result.sources()) {
            CalibrationCellKey forward_key{point, source, MotorDirection::Forward};
            CalibrationCellKey backward_key{point, source, MotorDirection::Backward};

            const auto forward_cell = result.cell(forward_key);
            const auto backward_cell = result.cell(backward_key);
            if (!forward_cell || !backward_cell || !forward_cell->angle() || !backward_cell->angle()) {
                continue;
            }

            CalibrationCellKey source_first{result.points().front(), source, MotorDirection::Forward};
            CalibrationCellKey source_last{result.points().back(), source, MotorDirection::Forward};
            const auto source_first_cell = result.cell(source_first);
            const auto source_last_cell = result.cell(source_last);
            if (!source_first_cell || !source_last_cell || !source_first_cell->angle() || !source_last_cell->angle()) {
                continue;
            }

            const double full_span = std::abs(*source_last_cell->angle() - *source_first_cell->angle());
            const double hysteresis_limit = full_span * precision_percent / 100.0;
            const float diff = std::abs(*forward_cell->angle() - *backward_cell->angle());

            if (diff > hysteresis_limit) {
                addHysteresisIssue(validation, forward_key, diff, hysteresis_limit);
                addHysteresisIssue(validation, backward_key, diff, hysteresis_limit);
            }
        }
    }

    current_validation_ = std::move(validation);
    notifyObservers();
}

void ClibrationResultValidationSource::notifyObservers() const {
    if (!current_validation_) {
        return;
    }
    observers_.notify([this](domain::ports::IClibrationResultObserver& observer) {
        observer.onClibrationResultValidationUpdated(*current_validation_);
    });
}

double ClibrationResultValidationSource::hysteresisLimitPercent() const {
    return deps_.info_settings.selectedPrecisionValue();
}

bool ClibrationResultValidationSource::isKuEnabled() const {
    return deps_.calibration_control.is_ku_enabled.get_copy();
}
