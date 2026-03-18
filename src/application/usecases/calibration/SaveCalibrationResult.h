#ifndef CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
#define CLEANGRADUATOR_SAVECALIBRATIONRESULT_H

#include <filesystem>
#include <optional>
#include <string>

#include "application/models/info/BatchContext.h"
#include "application/ports/batch/IBatchContextProvider.h"
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/core/calibration/result/CalibrationResult.h"
#include "domain/ports/calibration/result/ICalibrationResultSource.h"

namespace application::usecase {

class SaveCalibrationResult {
public:
    struct Result {
        bool success;
        std::string error;
        std::optional<models::BatchContext> batch;
        std::filesystem::path target_directory;
    };

    SaveCalibrationResult(
        domain::ports::ICalibrationResultSource& result_source,
        ports::ICalibrationResultSaver& saver,
        ports::IBatchContextProvider& batch_context_provider);

    std::optional<domain::common::CalibrationResult> currentResult() const;
    std::optional<models::BatchContext> previewNextBatch();
    Result save();
    Result saveAs(const std::filesystem::path& directory);

private:
    Result saveToDirectory(const std::filesystem::path& directory, std::optional<models::BatchContext> batch);

    domain::ports::ICalibrationResultSource& result_source_;
    ports::ICalibrationResultSaver& saver_;
    ports::IBatchContextProvider& batch_context_provider_;
};

}

#endif //CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
