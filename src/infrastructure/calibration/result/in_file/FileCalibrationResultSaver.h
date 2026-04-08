#ifndef CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#define CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#include <filesystem>
#include <vector>

#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/fmt/Logger.h"
#include "infrastructure/calibration/result/CalibrationResultSaverPorts.h"

namespace infra::calib {
    class FileCalibrationResultSaver final : public application::ports::ICalibrationResultSaver {
    public:
        explicit FileCalibrationResultSaver(CalibrationResultSaverPorts ports);

        Result save(const domain::common::CalibrationResult& result,
                    const std::filesystem::path& directory,
                    const std::vector<domain::common::SourceId>& selected_sources) override;

    private:
        fmt::Logger logger_;
    };
}


#endif //CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
