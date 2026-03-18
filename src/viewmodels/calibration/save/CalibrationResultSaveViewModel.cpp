#include "CalibrationResultSaveViewModel.h"

using namespace mvvm;

namespace {
const char* toStatusText(CalibrationResultSaveViewModel::SaveState state)
{
    switch (state) {
    case CalibrationResultSaveViewModel::SaveState::Saving:
        return "Сохраняется";
    case CalibrationResultSaveViewModel::SaveState::Saved:
        return "Сохранено";
    case CalibrationResultSaveViewModel::SaveState::Error:
        return "Ошибка";
    case CalibrationResultSaveViewModel::SaveState::NotSaved:
    default:
        return "Не сохранено";
    }
}
}

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(
    CalibrationResultSaveViewModelDeps deps)
    : control_(deps.control)
    , result_source_(deps.result_source)
{
    result_source_.addObserver(*this);
    refreshPreview();
    setState(SaveState::NotSaved);
    can_save.set(control_.hasResult());
    can_save_as.set(control_.hasResult());
    can_show_in_explorer.set(control_.lastSavedPath().has_value());
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel()
{
    result_source_.removeObserver(*this);
}

void CalibrationResultSaveViewModel::save()
{
    if (!control_.hasResult()) {
        setState(SaveState::Error, "Нет результата для сохранения.");
        return;
    }

    setState(SaveState::Saving);
    handleSaveResult(control_.save());
}

void CalibrationResultSaveViewModel::saveAs(const std::filesystem::path& output_path)
{
    if (!control_.hasResult()) {
        setState(SaveState::Error, "Нет результата для сохранения.");
        return;
    }

    setState(SaveState::Saving);
    handleSaveResult(control_.saveAs(output_path));
}

void CalibrationResultSaveViewModel::showInExplorer()
{
    const auto result = control_.showLastSavedInExplorer();
    if (!result.success) {
        setState(SaveState::Error, result.error);
    }
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&)
{
    refreshPreview();
    setState(SaveState::NotSaved);
    can_save.set(true);
    can_save_as.set(true);
}

void CalibrationResultSaveViewModel::refreshPreview()
{
    const auto preview = control_.previewNextBatch();
    party_id.set(preview ? preview->party_id : 0);
    can_save.set(control_.hasResult());
    can_save_as.set(control_.hasResult());
    can_show_in_explorer.set(control_.lastSavedPath().has_value());
}

void CalibrationResultSaveViewModel::setState(SaveState state, std::string err)
{
    save_state.set(state);
    status_text.set(toStatusText(state));
    error_text.set(std::move(err));
}

void CalibrationResultSaveViewModel::handleSaveResult(const application::ports::ICalibrationResultSaver::Result& result)
{
    if (!result.success) {
        setState(SaveState::Error, result.error);
        can_show_in_explorer.set(control_.lastSavedPath().has_value());
        return;
    }

    if (const auto preview = control_.previewNextBatch()) {
        if (const auto saved_party = party_id.get_copy(); saved_party > 0) {
            party_id.set(saved_party);
        } else {
            party_id.set(preview->party_id);
        }
    }
    setState(SaveState::Saved);
    can_show_in_explorer.set(control_.lastSavedPath().has_value());
}
