#ifndef CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLERPORTS_H
#define CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLERPORTS_H

namespace domain::ports {
    struct ILogger;
    struct ICalibrationLifecycle;
}

namespace application::orchestrators {
    class CalibrationSettingsQuery;

    struct CalibrationSessionControllerPorts {
        domain::ports::ILogger& logger;
        CalibrationSettingsQuery& settings_query;
        domain::ports::ICalibrationLifecycle& lifecycle;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONSESSIONCONTROLLERPORTS_H
