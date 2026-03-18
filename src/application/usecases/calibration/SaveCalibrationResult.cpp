#include "SaveCalibrationResult.h"

#include "application/ports/batch/IBatchContextProvider.h"
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace application::usecase {

SaveCalibrationResult::SaveCalibrationResult(SaveCalibrationResultDeps deps)
    : result_source_(deps.result_source)
    , result_saver_(deps.result_saver)
    , batch_context_provider_(deps.batch_context_provider)
{
}

SaveCalibrationResult::Preview SaveCalibrationResult::preview() const
{
    const auto batch_context = batch_context_provider_.previewNext();
    return {.next_party_id = batch_context ? std::make_optional(batch_context->party_id) : std::nullopt};
}

SaveCalibrationResult::Result SaveCalibrationResult::save() const
{
    const auto current_result = result_source_.currentResult();
    if (!current_result) {
        return {false, "Нет результата для сохранения", std::nullopt, std::nullopt};
    }

    const auto save_result = result_saver_.save(*current_result);
    return {save_result.success, save_result.error, save_result.party_id, save_result.directory};
}

SaveCalibrationResult::Result SaveCalibrationResult::saveToDirectory(const std::filesystem::path& directory) const
{
    const auto current_result = result_source_.currentResult();
    if (!current_result) {
        return {false, "Нет результата для сохранения", std::nullopt, std::nullopt};
    }

    const auto save_result = result_saver_.saveToDirectory(*current_result, directory);
    return {save_result.success, save_result.error, save_result.party_id, save_result.directory};
}

} // namespace application::usecase
