#include "CalibrationResultSaveViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace mvvm {

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps)
    : save_use_case_(deps.save_use_case)
    , result_source_(deps.result_source)
{
    result_source_.addObserver(*this);
    refresh();
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel() {
    result_source_.removeObserver(*this);
}

void CalibrationResultSaveViewModel::save() {
    publishState();
    save_use_case_.save();
    publishState();
}

void CalibrationResultSaveViewModel::saveAs(const std::filesystem::path& directory) {
    publishState();
    save_use_case_.saveAs(directory);
    publishState();
}

void CalibrationResultSaveViewModel::revealInExplorer() {
    const auto result = save_use_case_.revealInExplorer();
    if (!result.success) {
        error_text.set(result.error);
    } else {
        error_text.set("");
    }
    publishState();
}

void CalibrationResultSaveViewModel::refresh() {
    publishState();
}

void CalibrationResultSaveViewModel::publishState() {
    const auto state = save_use_case_.state();

    batch_number.set(state.batch_context.has_value()
        ? std::optional<int>{state.batch_context->party_id}
        : std::nullopt);

    const auto vm_status = mapStatus(state.status);
    status.set(vm_status);
    status_text.set(statusText(vm_status));
    error_text.set(state.error);
    can_save.set(state.has_result);
    can_reveal.set(state.can_reveal);
}

CalibrationResultSaveViewModel::Status
CalibrationResultSaveViewModel::mapStatus(application::usecase::SaveCalibrationResult::Status status) {
    switch (status) {
        case application::usecase::SaveCalibrationResult::Status::Unsaved:
            return Status::Unsaved;
        case application::usecase::SaveCalibrationResult::Status::Saving:
            return Status::Saving;
        case application::usecase::SaveCalibrationResult::Status::Saved:
            return Status::Saved;
        case application::usecase::SaveCalibrationResult::Status::Error:
            return Status::Error;
    }

    return Status::Unsaved;
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&) {
    publishState();
}

std::string CalibrationResultSaveViewModel::statusText(Status status) {
    switch (status) {
        case Status::Unsaved:
            return "Не сохранено";
        case Status::Saving:
            return "Сохраняется";
        case Status::Saved:
            return "Сохранено";
        case Status::Error:
            return "Ошибка";
    }

    return "Не сохранено";
}

}
