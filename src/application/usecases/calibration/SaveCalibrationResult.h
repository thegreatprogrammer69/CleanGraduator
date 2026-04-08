#ifndef CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
#define CLEANGRADUATOR_SAVECALIBRATIONRESULT_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "application/common/info/BatchContext.h"
#include "domain/core/angle/SourceId.h"

namespace application::ports {
class IBatchContextProvider;
class ICalibrationResultSaver;
}

namespace domain::ports {
class ICalibrationResultSource;
}

namespace application::usecase {

class SaveCalibrationResult {
public:
    struct Result {
        bool success;
        std::string error;
        std::optional<application::models::BatchContext> batch;
        std::filesystem::path saved_to;
    };

    SaveCalibrationResult(domain::ports::ICalibrationResultSource& result_source,
                          application::ports::ICalibrationResultSaver& saver,
                          application::ports::IBatchContextProvider& batch_context_provider);

    Result save(const std::vector<domain::common::SourceId>& selected_sources = {});
    Result saveAs(const std::filesystem::path& directory,
                  const std::vector<domain::common::SourceId>& selected_sources = {});

    const std::optional<application::models::BatchContext>& lastBatch() const;
    const std::filesystem::path& lastSavedPath() const;

private:
    Result saveToDirectory(const std::filesystem::path& directory,
                           std::optional<application::models::BatchContext> batch,
                           const std::vector<domain::common::SourceId>& selected_sources);

    domain::ports::ICalibrationResultSource& result_source_;
    application::ports::ICalibrationResultSaver& saver_;
    application::ports::IBatchContextProvider& batch_context_provider_;
    std::optional<application::models::BatchContext> last_batch_;
    std::filesystem::path last_saved_path_;
};

}

#endif //CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
