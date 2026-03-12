#ifndef CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#define CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H
#include "application/ports/calibration/result/ICalibrationResultSaver.h"
#include "infrastructure/calibration/result/CalibrationResultSaverPorts.h"

namespace infra::calib {
    class FileCalibrationResultSaver : public application::ports::ICalibrationResultSaver {
    public:
        explicit FileCalibrationResultSaver(CalibrationResultSaverPorts ports);

        Result save(const domain::common::CalibrationResult& result) override;

    private:
        CalibrationResultSaverPorts ports_;
    };

}
}


#endif //CLEANGRADUATOR_FILECALIBRATIONRESULTSAVER_H