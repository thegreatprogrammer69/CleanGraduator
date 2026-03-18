#ifndef CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#define CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/fmt/Logger.h"
#include "infrastructure/calibration/result/CalibrationResultSaverPorts.h"

namespace infra::calib {
    class FileCalibrationResultSaver final : public application::ports::ICalibrationResultSaver {
    public:
        explicit FileCalibrationResultSaver(CalibrationResultSaverPorts ports);

        Result save(const domain::common::CalibrationResult& result) override;
        Result saveAs(const domain::common::CalibrationResult& result, const std::filesystem::path& output_path) override;

    private:
        Result writeToPath(const domain::common::CalibrationResult& result, const std::filesystem::path& output_path);

        fmt::Logger logger_;
        application::ports::IBatchContextProvider& batch_context_provider_;
    };
}


#endif //CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
