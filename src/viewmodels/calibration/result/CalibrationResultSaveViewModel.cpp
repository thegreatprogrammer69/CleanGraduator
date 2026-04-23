#include "CalibrationResultSaveViewModel.h"

#include <algorithm>
#include <unordered_set>

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace mvvm {

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps)
    : save_use_case_(deps.save_use_case)
    , result_source_(deps.result_source)
    , validation_source_(deps.validation_source)
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

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::save()
{
    const auto camera_ids = availableCameraIds();
    return save(camera_ids);
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::saveWithoutErrors()
{
    const auto current = result_source_.currentResult();
    if (!current) {
        return save({});
    }

    std::unordered_set<int> blocked_sources;
    if (const auto validation = validation_source_.currentValidation()) {
        for (const auto& [key, issues] : validation->allIssues()) {
            const bool has_error = std::any_of(issues.begin(), issues.end(), [](const auto& issue) {
                return issue.severity == domain::common::CalibrationIssueSeverity::Error;
            });
            if (has_error) {
                blocked_sources.insert(key.source_id.value);
            }
        }
    }

    std::vector<int> selected;
    for (const auto& source : current->sources()) {
        if (blocked_sources.find(source.value) != blocked_sources.end()) {
            selected.push_back(source.value);
        }
    }

    std::sort(selected.begin(), selected.end());
    selected.erase(std::unique(selected.begin(), selected.end()), selected.end());
    return save(selected);
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::save(const std::vector<int>& camera_ids)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    std::vector<domain::common::SourceId> selected_sources;
    selected_sources.reserve(camera_ids.size());
    for (const int camera_id : camera_ids) {
        selected_sources.emplace_back(camera_id);
    }

    const auto result = save_use_case_.save(selected_sources);
    applySaveResult(result);
    return result;
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::saveAs(
    const std::filesystem::path& directory)
{
    const auto camera_ids = availableCameraIds();
    return saveAs(directory, camera_ids);
}

application::usecase::SaveCalibrationResult::Result CalibrationResultSaveViewModel::saveAs(
    const std::filesystem::path& directory,
    const std::vector<int>& camera_ids)
{
    save_state.set(CalibrationResultSaveState::Saving);
    save_state_text.set("Сохраняется");
    error_text.set("");

    std::vector<domain::common::SourceId> selected_sources;
    selected_sources.reserve(camera_ids.size());
    for (const int camera_id : camera_ids) {
        selected_sources.emplace_back(camera_id);
    }

    const auto result = save_use_case_.saveAs(directory, selected_sources);
    applySaveResult(result);
    return result;
}

bool CalibrationResultSaveViewModel::canRevealInExplorer() const
{
    return !save_use_case_.lastSavedPath().empty();
}

std::vector<int> CalibrationResultSaveViewModel::availableCameraIds() const
{
    std::vector<int> camera_ids;
    const auto current = result_source_.currentResult();
    if (!current) {
        return camera_ids;
    }

    camera_ids.reserve(current->sources().size());
    for (const auto& source_id : current->sources()) {
        camera_ids.push_back(source_id.value);
    }
    std::sort(camera_ids.begin(), camera_ids.end());
    camera_ids.erase(std::unique(camera_ids.begin(), camera_ids.end()), camera_ids.end());
    return camera_ids;
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
