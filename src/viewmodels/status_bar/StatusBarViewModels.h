#ifndef CLEANGRADUATOR_STATUSBARVIEWMODELS_H
#define CLEANGRADUATOR_STATUSBARVIEWMODELS_H

namespace mvvm {
    class AppStatusBarViewModel;
    class MotorDriverStatusViewModel;
    class PressureSensorStatusBarViewModel;

    struct StatusBarViewModels {
        AppStatusBarViewModel& app_status_bar;
        MotorDriverStatusViewModel& motor_driver_status;
        PressureSensorStatusBarViewModel& pressure_sensor_status;
    };
}

#endif //CLEANGRADUATOR_STATUSBARVIEWMODELS_H
