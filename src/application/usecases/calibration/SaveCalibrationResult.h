#ifndef CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
#define CLEANGRADUATOR_SAVECALIBRATIONRESULT_H

#include <filesystem>
#include <optional>
#include <string>

namespace application::ports {
    class ICalibrationResultSaver;
    struct IBatchContextProvider;
}

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace application::usecase {

    struct SaveCalibrationResultDeps {
        domain::ports::ICalibrationResultSource& result_source;
        application::ports::ICalibrationResultSaver& result_saver;
        application::ports::IBatchContextProvider& batch_context_provider;
    };

    class SaveCalibrationResult {
    public:
        struct Preview {
            std::optional<int> next_party_id;
        };

        struct Result {
            bool success;
            std::string error;
            std::optional<int> party_id;
            std::optional<std::filesystem::path> directory;
        };

        explicit SaveCalibrationResult(SaveCalibrationResultDeps deps);

        Preview preview() const;
        Result save() const;
        Result saveToDirectory(const std::filesystem::path& directory) const;

    private:
        domain::ports::ICalibrationResultSource& result_source_;
        application::ports::ICalibrationResultSaver& result_saver_;
        application::ports::IBatchContextProvider& batch_context_provider_;
    };
}

#endif //CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
