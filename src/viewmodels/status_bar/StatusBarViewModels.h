#ifndef CLEANGRADUATOR_STATUSBARVIEWMODELS_H
#define CLEANGRADUATOR_STATUSBARVIEWMODELS_H

namespace mvvm {
    class AppStatusBarViewModel;
    class MotorDriverStatusViewModel;

    struct StatusBarViewModels {
        AppStatusBarViewModel& app_status_bar;
        MotorDriverStatusViewModel& motor_driver_status;
    };
}

#endif //CLEANGRADUATOR_STATUSBARVIEWMODELS_H