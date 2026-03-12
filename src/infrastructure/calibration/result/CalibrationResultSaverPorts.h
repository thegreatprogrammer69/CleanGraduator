#ifndef CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"

namespace infra::calib {
    struct CalibrationResultSaverPorts {
        application::orchestrators::CalibrationSettingsQuery& settings_query_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONRESULTSAVERPORTS_H