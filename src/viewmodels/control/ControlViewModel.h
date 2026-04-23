#ifndef CLEANGRADUATOR_CONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CONTROLVIEWMODEL_H


namespace mvvm {
    class DualValveControlViewModel;
    class MotorControlViewModel;
    class CalibrationSessionControlViewModel;
    class CalibrationResultSaveViewModel;
    class AppStatusBarViewModel;
    class PressureSensorStatusBarViewModel;

    struct ControlViewModelDeps {
        DualValveControlViewModel& valves_vm;
        MotorControlViewModel& motor_vm;
        CalibrationSessionControlViewModel& calibration_vm;
        CalibrationResultSaveViewModel& calibration_result_save_vm;
        AppStatusBarViewModel& app_status_vm;
        PressureSensorStatusBarViewModel& pressure_sensor_vm;
    };

    class ControlViewModel {
    public:
        explicit ControlViewModel(ControlViewModelDeps deps);
        MotorControlViewModel& motorViewModel();
        DualValveControlViewModel& valvesViewModel();
        CalibrationSessionControlViewModel& calibrationViewModel();
        CalibrationResultSaveViewModel& calibrationResultSaveViewModel();
        AppStatusBarViewModel& appStatusViewModel();
        PressureSensorStatusBarViewModel& pressureSensorViewModel();

    private:
        MotorControlViewModel& motor_vm_;
        DualValveControlViewModel& valves_vm_;
        CalibrationSessionControlViewModel& calibration_vm_;
        CalibrationResultSaveViewModel& calibration_result_save_vm_;
        AppStatusBarViewModel& app_status_vm_;
        PressureSensorStatusBarViewModel& pressure_sensor_vm_;
    };

}


#endif //CLEANGRADUATOR_CONTROLVIEWMODEL_H
