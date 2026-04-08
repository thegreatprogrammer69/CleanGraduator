#include "CalibrationResultSaveViewModel.h"

#include <algorithm>

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
}

void CalibrationResultSaveViewModel::save()
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    applySaveResult(save_use_case_.save());
}

void CalibrationResultSaveViewModel::saveAs(const std::filesystem::path& directory)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    applySaveResult(save_use_case_.saveAs(directory));
}

CalibrationResultSaveViewModel::SaveActionFeedback
CalibrationResultSaveViewModel::saveForCameras(const std::vector<int>& camera_numbers)
{
    std::vector<domain::common::SourceId> source_ids;
    source_ids.reserve(camera_numbers.size());
    for (const auto camera : camera_numbers) {
        source_ids.emplace_back(camera);
    }

    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    const auto result = save_use_case_.saveForSources(source_ids);
    applySaveResult(result);

    return {result.success, result.error, result.saved_to};
}

bool CalibrationResultSaveViewModel::canRevealInExplorer() const
{
    return !save_use_case_.lastSavedPath().empty();
}

std::vector<int> CalibrationResultSaveViewModel::cameraNumbers() const
{
    std::vector<int> cameras;
    const auto& result = result_source_.currentResult();
    if (!result) {
        return cameras;
    }

    cameras.reserve(result->sources().size());
    for (const auto& source : result->sources()) {
        cameras.push_back(source.value);
    }

    std::sort(cameras.begin(), cameras.end());
    return cameras;
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
