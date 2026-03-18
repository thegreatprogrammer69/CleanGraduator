#include "CalibrationResultSaveControl.h"

using namespace application::usecase;

CalibrationResultSaveControl::CalibrationResultSaveControl(
    domain::ports::ICalibrationResultSource& result_source,
    application::ports::ICalibrationResultSaver& saver,
    application::ports::IBatchContextProvider& batch_context_provider,
    application::ports::IFileExplorerLauncher& file_explorer)
    : result_source_(result_source)
    , saver_(saver)
    , batch_context_provider_(batch_context_provider)
    , file_explorer_(file_explorer) {}

bool CalibrationResultSaveControl::hasResult() const
{
    return currentResult() != nullptr;
}

std::optional<application::models::BatchContext>
CalibrationResultSaveControl::previewNextBatch() const
{
    return batch_context_provider_.previewNext();
}

std::optional<std::filesystem::path> CalibrationResultSaveControl::lastSavedPath() const
{
    return last_saved_path_;
}

application::ports::ICalibrationResultSaver::Result CalibrationResultSaveControl::save()
{
    const auto* result = currentResult();
    if (!result) {
        return {false, "Нет результата для сохранения.", {}};
    }

    auto save_result = saver_.save(*result);
    if (save_result.success) {
        last_saved_path_ = save_result.output_path;
    }
    return save_result;
}

application::ports::ICalibrationResultSaver::Result CalibrationResultSaveControl::saveAs(
    const std::filesystem::path& output_path)
{
    const auto* result = currentResult();
    if (!result) {
        return {false, "Нет результата для сохранения.", {}};
    }

    auto save_result = saver_.saveAs(*result, output_path);
    if (save_result.success) {
        last_saved_path_ = save_result.output_path;
    }
    return save_result;
}

application::ports::IFileExplorerLauncher::Result
CalibrationResultSaveControl::showLastSavedInExplorer() const
{
    if (!last_saved_path_) {
        return {false, "Путь к сохранённой партии ещё не определён."};
    }

    return file_explorer_.showInExplorer(*last_saved_path_);
}

const domain::common::CalibrationResult* CalibrationResultSaveControl::currentResult() const
{
    const auto& result = result_source_.currentResult();
    return result ? &(*result) : nullptr;
}
