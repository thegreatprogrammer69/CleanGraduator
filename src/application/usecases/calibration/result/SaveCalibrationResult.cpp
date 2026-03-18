#include "SaveCalibrationResult.h"

#include <filesystem>

#include "application/ports/batch/IBatchContextProvider.h"
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "application/ports/system/IDesktopServices.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"

using namespace application::usecase;

SaveCalibrationResult::SaveCalibrationResult(SaveCalibrationResultDeps deps)
    : deps_(deps) {}

SaveCalibrationResult::Preview SaveCalibrationResult::preview() const {
    const auto& current_result = deps_.result_source.currentResult();
    if (!current_result) {
        return {false, 0, {}, "Нет результата для сохранения"};
    }

    if (!current_result->isReady()) {
        return {false, 0, {}, "Результат ещё не готов к сохранению"};
    }

    const auto batch_context = deps_.batch_context_provider.previewNext();
    if (!batch_context) {
        return {false, 0, {}, "Не удалось определить следующую партию"};
    }

    return {true, batch_context->party_id, batch_context->full_path, {}};
}

SaveCalibrationResult::Result SaveCalibrationResult::execute() {
    const auto batch_context = deps_.batch_context_provider.allocateNext();
    if (!batch_context) {
        return {false, std::nullopt, "Не удалось создать папку для сохранения партии"};
    }

    return saveToDirectory(batch_context->full_path, batch_context);
}

SaveCalibrationResult::Result SaveCalibrationResult::saveAs(const std::filesystem::path& directory) {
    if (directory.empty()) {
        return {false, std::nullopt, "Не выбрана папка для сохранения"};
    }

    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec) {
        return {false, std::nullopt, "Не удалось создать выбранную папку"};
    }

    return saveToDirectory(directory, std::nullopt);
}

application::ports::IDesktopServices::Result SaveCalibrationResult::reveal(const std::filesystem::path& path) {
    if (path.empty()) {
        return {false, "Папка для открытия ещё не определена"};
    }
    return deps_.desktop_services.revealInFileManager(path);
}

SaveCalibrationResult::Result SaveCalibrationResult::saveToDirectory(
    const std::filesystem::path& directory,
    std::optional<application::models::BatchContext> batch_context)
{
    const auto& current_result = deps_.result_source.currentResult();
    if (!current_result) {
        return {false, std::nullopt, "Нет результата для сохранения"};
    }

    if (!current_result->isReady()) {
        return {false, std::nullopt, "Результат ещё не готов к сохранению"};
    }

    const auto save_result = deps_.result_saver.save(*current_result, directory);
    if (!save_result.success) {
        return {false, std::nullopt, save_result.error};
    }

    return {true, std::move(batch_context), {}};
}
