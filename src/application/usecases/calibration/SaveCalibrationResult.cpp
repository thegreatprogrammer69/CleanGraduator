#include "SaveCalibrationResult.h"

#include "application/ports/batch/IBatchContextProvider.h"
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace application::usecase {

SaveCalibrationResult::SaveCalibrationResult(
    domain::ports::ICalibrationResultSource& result_source,
    application::ports::ICalibrationResultSaver& saver,
    application::ports::IBatchContextProvider& batch_context_provider)
    : result_source_(result_source)
    , saver_(saver)
    , batch_context_provider_(batch_context_provider)
{
}

SaveCalibrationResult::Result SaveCalibrationResult::save()
{
    const auto batch = batch_context_provider_.current();
    if (!batch) {
        return {false, "Не удалось подготовить директорию партии.", std::nullopt, {}};
    }

    return saveToDirectory(batch->full_path, batch);
}

SaveCalibrationResult::Result SaveCalibrationResult::saveAs(const std::filesystem::path& directory)
{
    if (directory.empty()) {
        return {false, "Не выбрана директория для сохранения.", last_batch_, last_saved_path_};
    }

    return saveToDirectory(directory, std::nullopt);
}

SaveCalibrationResult::Result SaveCalibrationResult::saveForSources(
    const std::vector<domain::common::SourceId>& source_ids)
{
    if (source_ids.empty()) {
        return {false, "Не выбрана ни одна камера для сохранения.", last_batch_, last_saved_path_};
    }

    const auto batch = batch_context_provider_.current();
    if (!batch) {
        return {false, "Не удалось подготовить директорию партии.", std::nullopt, {}};
    }

    return saveToDirectory(batch->full_path, batch, source_ids);
}

const std::optional<application::models::BatchContext>& SaveCalibrationResult::lastBatch() const
{
    return last_batch_;
}

const std::filesystem::path& SaveCalibrationResult::lastSavedPath() const
{
    return last_saved_path_;
}

SaveCalibrationResult::Result SaveCalibrationResult::saveToDirectory(
    const std::filesystem::path& directory,
    std::optional<application::models::BatchContext> batch,
    const std::optional<std::vector<domain::common::SourceId>>& source_ids)
{
    const auto& current_result = result_source_.currentResult();
    if (!current_result) {
        return {false, "Нет результата для сохранения.", last_batch_, last_saved_path_};
    }

    const auto save_result = saver_.save(*current_result, directory, source_ids);
    if (!save_result.success) {
        return {false, save_result.error, last_batch_, last_saved_path_};
    }

    last_batch_ = std::move(batch);
    last_saved_path_ = save_result.saved_to.empty() ? directory : save_result.saved_to;

    return {true, "", last_batch_, last_saved_path_};
}

} // namespace application::usecase
