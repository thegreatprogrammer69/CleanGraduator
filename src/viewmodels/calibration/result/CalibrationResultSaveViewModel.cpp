#include "CalibrationResultSaveViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace mvvm {

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps)
    : save_use_case_(deps.save_use_case)
    , result_source_(deps.result_source)
{
    result_source_.addObserver(*this);

    const auto current_result = result_source_.currentResult();
    const bool has_result = current_result.has_value();
    can_save.set(has_result);
    can_save_as.set(has_result);
    can_show_in_explorer.set(canRevealInExplorer());
    if (current_result) {
        std::vector<int> cameras;
        cameras.reserve(current_result->sources().size());
        for (const auto& source_id : current_result->sources()) {
            cameras.push_back(source_id.value);
        }
        camera_ids.set(cameras);
    } else {
        camera_ids.set({});
    }
    updateBatchInfo();
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel()
{
    result_source_.removeObserver(*this);
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&)
{
    const auto current_result = result_source_.currentResult();
    std::vector<int> cameras;
    if (current_result) {
        cameras.reserve(current_result->sources().size());
        for (const auto& source_id : current_result->sources()) {
            cameras.push_back(source_id.value);
        }
    }
    camera_ids.set(cameras);

    save_state.set(CalibrationResultSaveState::NotSaved);
    save_state_text.set("Не сохранено");
    error_text.set("");
    can_save.set(true);
    can_save_as.set(true);
    can_show_in_explorer.set(canRevealInExplorer());
}

void CalibrationResultSaveViewModel::save(const std::vector<int>& selected_camera_ids)
{
    saveAndGetResult(selected_camera_ids);
}

application::usecase::SaveCalibrationResult::Result
CalibrationResultSaveViewModel::saveAndGetResult(const std::vector<int>& selected_camera_ids)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    const auto result = save_use_case_.save(toSourceIds(selected_camera_ids));
    applySaveResult(result);
    return result;
}

void CalibrationResultSaveViewModel::saveAs(
    const std::filesystem::path& directory,
    const std::vector<int>& selected_camera_ids)
{
    saveAsAndGetResult(directory, selected_camera_ids);
}

application::usecase::SaveCalibrationResult::Result
CalibrationResultSaveViewModel::saveAsAndGetResult(
    const std::filesystem::path& directory,
    const std::vector<int>& selected_camera_ids)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    const auto result = save_use_case_.saveAs(directory, toSourceIds(selected_camera_ids));
    applySaveResult(result);
    return result;
}

bool CalibrationResultSaveViewModel::canRevealInExplorer() const
{
    return !save_use_case_.lastSavedPath().empty();
}

std::vector<int> CalibrationResultSaveViewModel::availableCameraIds() const
{
    return camera_ids.get_copy();
}

std::vector<domain::common::SourceId> CalibrationResultSaveViewModel::toSourceIds(
    const std::vector<int>& camera_ids) const
{
    std::vector<domain::common::SourceId> result;
    result.reserve(camera_ids.size());
    for (int camera_id : camera_ids) {
        result.emplace_back(camera_id);
    }
    return result;
}

void CalibrationResultSaveViewModel::applySaveResult(const application::usecase::SaveCalibrationResult::Result& result)
{
    if (result.success) {
        save_state.set(CalibrationResultSaveState::Saved);
        save_state_text.set("Сохранено");
        error_text.set("");
        last_saved_path.set(result.saved_to);
        can_show_in_explorer.set(!result.saved_to.empty());
        updateBatchInfo();
        return;
    }

    save_state.set(CalibrationResultSaveState::Error);
    save_state_text.set("Ошибка");
    error_text.set(result.error);
    can_show_in_explorer.set(canRevealInExplorer());
    updateBatchInfo();
}

void CalibrationResultSaveViewModel::updateBatchInfo()
{
    const auto& batch = save_use_case_.lastBatch();
    if (!batch) {
        batch_number.set(0);
        batch_text.set("Партия № —");
        return;
    }

    batch_number.set(batch->party_id);
    batch_text.set("Партия № " + std::to_string(batch->party_id));
}

} // namespace mvvm
