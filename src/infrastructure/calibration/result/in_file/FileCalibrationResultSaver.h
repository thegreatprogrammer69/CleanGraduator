#ifndef CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#define CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "domain/fmt/Logger.h"
#include "infrastructure/calibration/result/CalibrationResultSaverPorts.h"

namespace infra::calib {
    class FileCalibrationResultSaver final : public application::ports::ICalibrationResultSaver {
    public:
        explicit FileCalibrationResultSaver(CalibrationResultSaverPorts ports);

        Result save(const domain::common::CalibrationResult& result,
                    const std::filesystem::path& directory) override;

    private:
        fmt::Logger logger_;
    };
}


#endif //CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
