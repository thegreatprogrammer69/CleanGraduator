#include "CalibrationResultSaveViewModel.h"

#include "application/usecases/calibration/result/SaveCalibrationResult.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"

using namespace mvvm;

namespace {
    mvvm::CalibrationResultSaveViewModel::ViewState make_state(
        int party_id,
        mvvm::CalibrationResultSaveViewModel::SaveState status,
        std::string status_text,
        std::string status_color,
        std::filesystem::path path,
        std::string last_error,
        bool can_save,
        bool can_show_in_explorer,
        bool can_save_as)
    {
        return {
            party_id,
            status,
            std::move(status_text),
            std::move(status_color),
            std::move(path),
            std::move(last_error),
            can_save,
            can_show_in_explorer,
            can_save_as
        };
    }
}

CalibrationResultSaveViewModel::CalibrationResultSaveViewModel(CalibrationResultSaveViewModelDeps deps)
    : deps_(deps)
{
    deps_.result_source.addObserver(*this);
    if (deps_.result_source.currentResult()) {
        current_revision_ = 1;
    }
    refreshState();
}

CalibrationResultSaveViewModel::~CalibrationResultSaveViewModel() {
    deps_.result_source.removeObserver(*this);
}

void CalibrationResultSaveViewModel::save() {
    auto current = makeBaseState();
    current.status = SaveState::Saving;
    current.status_text = "Сохраняется";
    current.status_color = "#d97706";
    current.last_error.clear();
    state.set(current, true);

    const auto result = deps_.save_result.execute();
    if (!result.success) {
        auto error_state = makeBaseState();
        error_state.status = SaveState::Error;
        error_state.status_text = "Ошибка";
        error_state.status_color = "#dc2626";
        error_state.last_error = result.error;
        state.set(error_state, true);
        return;
    }

    saved_revision_ = current_revision_;
    last_saved_batch_ = result.batch_context;
    if (result.batch_context) {
        last_saved_path_ = result.batch_context->full_path;
    }
    state.set(savedState(last_saved_batch_ ? last_saved_batch_->party_id : 0, last_saved_path_), true);
}

void CalibrationResultSaveViewModel::saveAs(const std::filesystem::path& directory) {
    const auto result = deps_.save_result.saveAs(directory);
    if (!result.success) {
        auto error_state = makeBaseState();
        error_state.status = SaveState::Error;
        error_state.status_text = "Ошибка";
        error_state.status_color = "#dc2626";
        error_state.last_error = result.error;
        state.set(error_state, true);
        return;
    }

    saved_revision_ = current_revision_;
    last_saved_batch_.reset();
    last_saved_path_ = directory;
    state.set(savedState(0, last_saved_path_), true);
}

void CalibrationResultSaveViewModel::showInExplorer() {
    const auto current = state.get_copy();
    const auto reveal_result = deps_.save_result.reveal(current.path);
    if (!reveal_result.success) {
        auto error_state = current;
        error_state.status = SaveState::Error;
        error_state.status_text = "Ошибка";
        error_state.status_color = "#dc2626";
        error_state.last_error = reveal_result.error;
        state.set(error_state, true);
    }
}

void CalibrationResultSaveViewModel::onCalibrationResultUpdated(const domain::common::CalibrationResult&) {
    ++current_revision_;
    if (saved_revision_ && *saved_revision_ != current_revision_) {
        last_saved_batch_.reset();
        last_saved_path_.clear();
    }
    refreshState();
}

void CalibrationResultSaveViewModel::refreshState() {
    if (saved_revision_ && *saved_revision_ == current_revision_ && !last_saved_path_.empty()) {
        state.set(savedState(last_saved_batch_ ? last_saved_batch_->party_id : 0, last_saved_path_), true);
        return;
    }

    state.set(makeBaseState(), true);
}

CalibrationResultSaveViewModel::ViewState CalibrationResultSaveViewModel::makeBaseState() const {
    const auto preview = deps_.save_result.preview();
    if (!preview.available) {
        return make_state(
            preview.party_id,
            SaveState::NotSaved,
            "Не сохранено",
            "#6b7280",
            {},
            preview.error,
            false,
            !last_saved_path_.empty(),
            deps_.result_source.currentResult().has_value() && deps_.result_source.currentResult()->isReady());
    }

    return make_state(
        preview.party_id,
        SaveState::NotSaved,
        "Не сохранено",
        "#6b7280",
        preview.path,
        {},
        true,
        !last_saved_path_.empty(),
        true);
}

CalibrationResultSaveViewModel::ViewState CalibrationResultSaveViewModel::savedState(
    int party_id,
    const std::filesystem::path& path,
    const std::string& error)
{
    return make_state(
        party_id,
        SaveState::Saved,
        "Сохранено",
        "#16a34a",
        path,
        error,
        true,
        !path.empty(),
        true);
}
