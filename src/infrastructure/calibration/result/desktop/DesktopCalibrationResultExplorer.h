#ifndef CLEANGRADUATOR_DESKTOPCALIBRATIONRESULTEXPLORER_H
#define CLEANGRADUATOR_DESKTOPCALIBRATIONRESULTEXPLORER_H
#include "application/ports/calibration/result/ICalibrationResultExplorer.h"

namespace infra::calib {

class DesktopCalibrationResultExplorer final : public application::ports::ICalibrationResultExplorer {
public:
    Result reveal(const std::filesystem::path& directory) override;
};

}

#endif //CLEANGRADUATOR_DESKTOPCALIBRATIONRESULTEXPLORER_H
