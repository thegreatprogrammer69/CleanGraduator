#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVECONTROL_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVECONTROL_H

#include <filesystem>
#include <optional>
#include <string>

#include "application/models/info/BatchContext.h"
#include "application/ports/batch/IBatchContextProvider.h"
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "application/ports/desktop/IFileExplorerLauncher.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace application::usecase {

class CalibrationResultSaveControl {
public:
    CalibrationResultSaveControl(
        domain::ports::ICalibrationResultSource& result_source,
        ports::ICalibrationResultSaver& saver,
        ports::IBatchContextProvider& batch_context_provider,
        ports::IFileExplorerLauncher& file_explorer);

    [[nodiscard]] bool hasResult() const;
    [[nodiscard]] std::optional<models::BatchContext> previewNextBatch() const;
    [[nodiscard]] std::optional<std::filesystem::path> lastSavedPath() const;

    ports::ICalibrationResultSaver::Result save();
    ports::ICalibrationResultSaver::Result saveAs(const std::filesystem::path& output_path);
    ports::IFileExplorerLauncher::Result showLastSavedInExplorer() const;

private:
    [[nodiscard]] const domain::common::CalibrationResult* currentResult() const;

    domain::ports::ICalibrationResultSource& result_source_;
    ports::ICalibrationResultSaver& saver_;
    ports::IBatchContextProvider& batch_context_provider_;
    ports::IFileExplorerLauncher& file_explorer_;
    std::optional<std::filesystem::path> last_saved_path_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVECONTROL_H
