#include "CalibrationResultSaveViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace mvvm {

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps)
    : save_use_case_(deps.save_use_case)
    , result_source_(deps.result_source)
{
    result_source_.addObserver(*this);

    const bool has_result = result_source_.currentResult().has_value();
    can_save.set(has_result);
    can_save_as.set(has_result);
    can_show_in_explorer.set(canRevealInExplorer());
    available_camera_ids.set(availableCameraIds());
    updateBatchInfo();
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel()
{
    result_source_.removeObserver(*this);
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&)
{
    save_state.set(CalibrationResultSaveState::NotSaved);
    save_state_text.set("Не сохранено");
    error_text.set("");
    can_save.set(true);
    can_save_as.set(true);
    can_show_in_explorer.set(canRevealInExplorer());
    available_camera_ids.set(availableCameraIds());
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::save()
{
    return save({});
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::save(const std::vector<int>& camera_ids)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    const auto result = save_use_case_.save(mapToSourceIds(camera_ids));
    applySaveResult(result);
    return result;
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::saveAs(
    const std::filesystem::path& directory)
{
    return saveAs(directory, {});
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::saveAs(
    const std::filesystem::path& directory,
    const std::vector<int>& camera_ids)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    const auto result = save_use_case_.saveAs(directory, mapToSourceIds(camera_ids));
    applySaveResult(result);
    return result;
}

bool CalibrationResultSaveViewModel::canRevealInExplorer() const
{
    return !save_use_case_.lastSavedPath().empty();
}

std::vector<int> CalibrationResultSaveViewModel::availableCameraIds() const
{
    std::vector<int> ids;
    const auto& result = result_source_.currentResult();
    if (!result) {
        return ids;
    }

    for (const auto& source : result->sources()) {
        ids.push_back(source.value);
    }
    return ids;
}

std::vector<domain::common::SourceId> CalibrationResultSaveViewModel::mapToSourceIds(
    const std::vector<int>& camera_ids) const
{
    std::vector<domain::common::SourceId> mapped;
    mapped.reserve(camera_ids.size());
    for (const auto camera_id : camera_ids) {
        mapped.emplace_back(camera_id);
    }
    return mapped;
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
