#include "CalibrationResultSaveViewModel.h"

#include "domain/ports/calibration/result/ICalibrationResultSource.h"

using namespace mvvm;

namespace {
    std::string color_for_state(CalibrationResultSaveViewModel::SaveState state)
    {
        switch (state) {
            case CalibrationResultSaveViewModel::SaveState::Saved:
                return "#15803d";
            case CalibrationResultSaveViewModel::SaveState::Saving:
                return "#b45309";
            case CalibrationResultSaveViewModel::SaveState::Unsaved:
                return "#6b7280";
            case CalibrationResultSaveViewModel::SaveState::Error:
                return "#b91c1c";
        }
        return "#6b7280";
    }

    std::string text_for_state(CalibrationResultSaveViewModel::SaveState state)
    {
        switch (state) {
            case CalibrationResultSaveViewModel::SaveState::Saved:
                return "Сохранено";
            case CalibrationResultSaveViewModel::SaveState::Saving:
                return "Сохраняется";
            case CalibrationResultSaveViewModel::SaveState::Unsaved:
                return "Не сохранено";
            case CalibrationResultSaveViewModel::SaveState::Error:
                return "Ошибка";
        }
        return "Не сохранено";
    }
}

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(
    CalibrationResultSaveViewModelDeps deps)
    : result_source_(deps.result_source)
    , save_result_(deps.save_result)
{
    result_source_.addObserver(*this);
    refreshDraftState();
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel()
{
    result_source_.removeObserver(*this);
}

void CalibrationResultSaveViewModel::save()
{
    if (!hasResult()) {
        applyState(SaveState::Error, "Нет результата для сохранения.");
        return;
    }

    applyState(SaveState::Saving);
    const auto save_result = save_result_.save();
    if (!save_result.success) {
        applyState(SaveState::Error, save_result.error);
        return;
    }

    draft_batch_ = save_result.batch;
    if (draft_batch_) {
        party_title.set("Партия № " + std::to_string(draft_batch_->party_id));
    }

    last_saved_directory.set(save_result.target_directory);
    applyState(SaveState::Saved);
}

void CalibrationResultSaveViewModel::saveAs(const std::filesystem::path& directory)
{
    if (directory.empty()) {
        return;
    }

    if (!hasResult()) {
        applyState(SaveState::Error, "Нет результата для сохранения.");
        return;
    }

    applyState(SaveState::Saving);
    const auto save_result = save_result_.saveAs(directory);
    if (!save_result.success) {
        applyState(SaveState::Error, save_result.error);
        return;
    }

    last_saved_directory.set(save_result.target_directory);
    applyState(SaveState::Saved);
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&)
{
    last_saved_directory.set(std::nullopt);
    refreshDraftState();
    applyState(SaveState::Unsaved);
}

void CalibrationResultSaveViewModel::refreshDraftState()
{
    if (const auto result = save_result_.currentResult(); result) {
        draft_batch_ = save_result_.previewNextBatch();
        if (draft_batch_) {
            party_title.set("Партия № " + std::to_string(draft_batch_->party_id));
        } else {
            party_title.set("Партия № —");
        }
    } else {
        draft_batch_.reset();
        party_title.set("Партия № —");
    }

    const bool available = hasResult();
    can_save.set(available && draft_batch_.has_value());
    can_save_as.set(available);
    can_show_in_explorer.set(last_saved_directory.get_copy().has_value());
}

void CalibrationResultSaveViewModel::applyState(SaveState state, const std::string& error)
{
    status_text.set(text_for_state(state));
    status_color.set(color_for_state(state));
    error_text.set(error);

    const bool available = hasResult();
    const bool can_save_now =
        state != SaveState::Saving &&
        state != SaveState::Saved &&
        available &&
        draft_batch_.has_value();
    can_save.set(can_save_now);
    can_save_as.set(state != SaveState::Saving && available);
    can_show_in_explorer.set(
        state != SaveState::Saving && last_saved_directory.get_copy().has_value());
}

bool CalibrationResultSaveViewModel::hasResult() const
{
    return result_source_.currentResult().has_value();
}
