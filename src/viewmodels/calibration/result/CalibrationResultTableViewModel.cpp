#include "CalibrationResultTableViewModel.h"

#include <variant>

#include "domain/ports/calibration/recording/ICalibrationRecorder.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"
#include "domain/ports/calibration/result/ICalibrationResultValidationSource.h"

using namespace mvvm;
using namespace domain::common;

CalibrationResultTableViewModel::CalibrationResultTableViewModel(CalibrationResultTableViewModelDeps deps)
    : recorder_(deps.recorder)
    , result_source_(deps.result_source)
    , validation_source_(deps.validation_source)
{
    recorder_.addObserver(*this);
    result_source_.addObserver(*this);
    validation_source_.addObserver(*this);
    info.set(info_);
    current_result.set(result_source_.currentResult());
    current_validation.set(validation_source_.currentValidation());
}

CalibrationResultTableViewModel::~CalibrationResultTableViewModel() {
    recorder_.removeObserver(*this);
    validation_source_.removeObserver(*this);
    result_source_.removeObserver(*this);
}

void CalibrationResultTableViewModel::onCalibrationRecorderEvent(const CalibrationRecorderEvent& ev) {
    std::visit([this](const auto& e) {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, CalibrationRecorderEvent::RecordingStarted>
                   || std::is_same_v<T, CalibrationRecorderEvent::RecordingStopped>) {
            resetInfo();
        } else if constexpr (std::is_same_v<T, CalibrationRecorderEvent::SessionStarted>) {
            active_session_id_ = e.id;
        } else if constexpr (std::is_same_v<T, CalibrationRecorderEvent::SessionEnded>) {
            if (active_session_id_ && *active_session_id_ == e.id) {
                active_session_id_.reset();
            }
        } else if constexpr (std::is_same_v<T, CalibrationRecorderEvent::AngleSampleRecorded>) {
            info_.current_angles[e.sample.id] = e.sample.angle;

            if (active_session_id_) {
                auto& bucket = active_session_id_->direction == MotorDirection::Forward
                    ? info_.forward_measurement_counts
                    : info_.backward_measurement_counts;
                bucket[e.sample.id] += 1;
            }

            info.set(info_, true);
        }
    }, ev.data);
}

void CalibrationResultTableViewModel::onCalibrationResultUpdated(const CalibrationResult &result) {
    current_result.set(result);
}


void CalibrationResultTableViewModel::onCalibrationResultValidationUpdated(const CalibrationResultValidation& validation) {
    current_validation.set(validation, true);
}

void CalibrationResultTableViewModel::resetInfo()
{
    active_session_id_.reset();
    info_ = {};
    info.set(info_, true);
}
