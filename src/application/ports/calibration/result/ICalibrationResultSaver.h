#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTSAVER_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTSAVER_H
#include <optional>
#include "domain/core/calibration/result/CalibrationResult.h"

namespace application::ports {

    class ICalibrationResultSaver {
    public:
        struct Result {
            bool success;
            std::string error;
        };
        virtual ~ICalibrationResultSaver() = default;
        virtual Result save(const domain::common::CalibrationResult& result) = 0;
    };

}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTSAVER_H