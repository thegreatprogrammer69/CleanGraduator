#ifndef CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
#define CLEANGRADUATOR_SAVECALIBRATIONRESULT_H

#include <filesystem>
#include <optional>
#include <string>

#include "application/models/info/BatchContext.h"
#include "application/ports/batch/IBatchContextProvider.h"
#include "application/ports/calibration/result/ICalibrationResultExplorer.h"
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/core/calibration/result/CalibrationResult.h"
#include "domain/ports/calibration/result/ICalibrationResultObserver.h"

namespace domain::ports {
class ICalibrationResultSource;
}

namespace application::usecase {

struct SaveCalibrationResultDeps {
    domain::ports::ICalibrationResultSource& result_source;
    application::ports::IBatchContextProvider& batch_context_provider;
    application::ports::IBatchContextAllocator& batch_context_allocator;
    application::ports::ICalibrationResultSaver& saver;
    application::ports::ICalibrationResultExplorer& explorer;
};

class SaveCalibrationResult final : public domain::ports::ICalibrationResultObserver {
public:
    enum class Status {
        Unsaved,
        Saving,
        Saved,
        Error,
    };

    struct State {
        std::optional<application::models::BatchContext> batch_context;
        Status status{Status::Unsaved};
        std::string error;
        bool has_result{false};
        bool can_reveal{false};
    };

    explicit SaveCalibrationResult(SaveCalibrationResultDeps deps);
    ~SaveCalibrationResult() override;

    State state() const;

    application::ports::ICalibrationResultSaver::Result save();
    application::ports::ICalibrationResultSaver::Result saveAs(const std::filesystem::path& directory);
    application::ports::ICalibrationResultExplorer::Result revealInExplorer();

    void onCalibrationResultUpdated(const domain::common::CalibrationResult& result) override;

private:
    application::ports::ICalibrationResultSaver::Result saveToDirectory(const std::filesystem::path& directory);
    void ensureBatchContext();

private:
    domain::ports::ICalibrationResultSource& result_source_;
    application::ports::IBatchContextProvider& batch_context_provider_;
    application::ports::IBatchContextAllocator& batch_context_allocator_;
    application::ports::ICalibrationResultSaver& saver_;
    application::ports::ICalibrationResultExplorer& explorer_;

    std::optional<domain::common::CalibrationResult> current_result_;
    Status status_{Status::Unsaved};
    std::string error_;
    bool can_reveal_{false};
    std::optional<std::filesystem::path> last_saved_directory_;
};

}

#endif //CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
