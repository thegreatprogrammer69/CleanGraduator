#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTSAVER_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTSAVER_H

#include <filesystem>
#include <string>
#include <vector>

#include "domain/core/calibration/result/CalibrationResult.h"
#include "domain/core/angle/SourceId.h"

namespace application::ports {

class ICalibrationResultSaver {
public:
    struct Result {
        bool success;
        std::string error;
        std::filesystem::path saved_to;
    };

    virtual ~ICalibrationResultSaver() = default;
    virtual Result save(const domain::common::CalibrationResult& result,
                        const std::filesystem::path& directory,
                        const std::vector<domain::common::SourceId>& selected_sources) = 0;
};

}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTSAVER_H
