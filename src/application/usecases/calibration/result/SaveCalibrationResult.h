#ifndef CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
#define CLEANGRADUATOR_SAVECALIBRATIONRESULT_H

#include <filesystem>
#include <optional>
#include <string>

#include "application/models/info/BatchContext.h"
#include "application/ports/system/IDesktopServices.h"

namespace application::ports {
    struct IBatchContextProvider;
    class ICalibrationResultSaver;
}

namespace domain::ports {
    class ICalibrationResultSource;
}

namespace application::usecase {
    struct SaveCalibrationResultDeps {
        domain::ports::ICalibrationResultSource& result_source;
        application::ports::IBatchContextProvider& batch_context_provider;
        application::ports::ICalibrationResultSaver& result_saver;
        application::ports::IDesktopServices& desktop_services;
    };

    class SaveCalibrationResult {
    public:
        struct Preview {
            bool available{false};
            int party_id{0};
            std::filesystem::path path;
            std::string error;
        };

        struct Result {
            bool success{false};
            std::optional<application::models::BatchContext> batch_context;
            std::string error;
        };

        explicit SaveCalibrationResult(SaveCalibrationResultDeps deps);

        Preview preview() const;
        Result execute();
        Result saveAs(const std::filesystem::path& directory);
        application::ports::IDesktopServices::Result reveal(const std::filesystem::path& path);

    private:
        Result saveToDirectory(const std::filesystem::path& directory,
                               std::optional<application::models::BatchContext> batch_context);

    private:
        SaveCalibrationResultDeps deps_;
    };
}

#endif //CLEANGRADUATOR_SAVECALIBRATIONRESULT_H
