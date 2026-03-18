#include "CalibrationResultSaveViewModel.h"

#include <QString>

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace mvvm {

namespace {
QString makePartyLabel(const std::optional<int>& party_id)
{
    if (!party_id) {
        return QStringLiteral("Партия № —");
    }
    return QStringLiteral("Партия № %1").arg(*party_id);
}
}

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps)
    : use_case_(deps.use_case)
    , result_source_(deps.result_source)
{
    result_source_.addObserver(*this);

    const auto has_result = result_source_.currentResult().has_value();
    can_save.set(has_result);
    can_save_as.set(has_result);
    refreshPreview();
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel()
{
    result_source_.removeObserver(*this);
}

void CalibrationResultSaveViewModel::save()
{
    if (!can_save.get_copy()) {
        return;
    }

    status_state.set(SaveState::Saving);
    status_text.set("Сохраняется");
    error_text.set("");

    applySaveResult(use_case_.save(), false);
}

void CalibrationResultSaveViewModel::saveAs(const std::filesystem::path& directory)
{
    if (!can_save_as.get_copy()) {
        return;
    }

    status_state.set(SaveState::Saving);
    status_text.set("Сохраняется");
    error_text.set("");

    applySaveResult(use_case_.saveToDirectory(directory), true);
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&)
{
    markUnsaved();
}

void CalibrationResultSaveViewModel::refreshPreview()
{
    const auto preview = use_case_.preview();
    party_label.set(makePartyLabel(preview.next_party_id).toStdString());
}

void CalibrationResultSaveViewModel::markUnsaved()
{
    const auto has_result = result_source_.currentResult().has_value();
    can_save.set(has_result);
    can_save_as.set(has_result);
    can_show_in_explorer.set(saved_directory.get_copy().has_value());

    status_state.set(SaveState::Unsaved);
    status_text.set(has_result ? "Не сохранено" : "Нет результата");
    error_text.set("");
    saved_directory.set(std::nullopt);
    can_show_in_explorer.set(false);
    refreshPreview();
}

void CalibrationResultSaveViewModel::applySaveResult(
    const application::usecase::SaveCalibrationResult::Result& result,
    const bool keep_preview_party)
{
    if (!result.success) {
        status_state.set(SaveState::Error);
        status_text.set("Ошибка");
        error_text.set(result.error);
        can_show_in_explorer.set(saved_directory.get_copy().has_value());
        if (!keep_preview_party) {
            refreshPreview();
        }
        return;
    }

    if (result.party_id) {
        party_label.set(makePartyLabel(result.party_id).toStdString());
    }

    status_state.set(SaveState::Saved);
    status_text.set("Сохранено");
    error_text.set("");
    saved_directory.set(result.directory);
    can_show_in_explorer.set(result.directory.has_value());
    can_save.set(true);
    can_save_as.set(true);

    if (!keep_preview_party) {
        // После обычного сохранения следующий превью-номер нужен только для нового результата,
        // поэтому оставляем номер только что сохранённой партии.
    }
}

} // namespace mvvm
