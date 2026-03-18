#include "SaveCalibrationResult.h"

using namespace application::usecase;

SaveCalibrationResult::SaveCalibrationResult(
    domain::ports::ICalibrationResultSource& result_source,
    ports::ICalibrationResultSaver& saver,
    ports::IBatchContextProvider& batch_context_provider)
    : result_source_(result_source)
    , saver_(saver)
    , batch_context_provider_(batch_context_provider)
{
}

std::optional<domain::common::CalibrationResult> SaveCalibrationResult::currentResult() const
{
    return result_source_.currentResult();
}

std::optional<application::models::BatchContext> SaveCalibrationResult::previewNextBatch()
{
    return batch_context_provider_.peekNext();
}

SaveCalibrationResult::Result SaveCalibrationResult::save()
{
    const auto batch = batch_context_provider_.createNext();
    if (!batch) {
        return { false, "Не удалось подготовить каталог партии для сохранения.", std::nullopt, {} };
    }

    return saveToDirectory(batch->full_path, batch);
}

SaveCalibrationResult::Result SaveCalibrationResult::saveAs(const std::filesystem::path& directory)
{
    return saveToDirectory(directory, std::nullopt);
}

SaveCalibrationResult::Result SaveCalibrationResult::saveToDirectory(
    const std::filesystem::path& directory,
    std::optional<application::models::BatchContext> batch)
{
    const auto result = result_source_.currentResult();
    if (!result) {
        return { false, "Нет результата для сохранения.", std::move(batch), directory };
    }

    const auto save_result = saver_.save(*result, directory);
    return { save_result.success, save_result.error, std::move(batch), directory };
}
