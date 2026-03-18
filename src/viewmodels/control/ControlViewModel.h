#ifndef CLEANGRADUATOR_CONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CONTROLVIEWMODEL_H


namespace mvvm {
    class DualValveControlViewModel;
    class MotorControlViewModel;
    class CalibrationSessionControlViewModel;
    class CalibrationResultSaveViewModel;

    struct ControlViewModelDeps {
        DualValveControlViewModel& valves_vm;
        MotorControlViewModel& motor_vm;
        CalibrationSessionControlViewModel& calibration_vm;
        CalibrationResultSaveViewModel& calibration_result_save_vm;
    };

    class ControlViewModel {
    public:
        explicit ControlViewModel(ControlViewModelDeps deps);
        MotorControlViewModel& motorViewModel();
        DualValveControlViewModel& valvesViewModel();
        CalibrationSessionControlViewModel& calibrationViewModel();
        CalibrationResultSaveViewModel& calibrationResultSaveViewModel();

    private:
        MotorControlViewModel& motor_vm_;
        DualValveControlViewModel& valves_vm_;
        CalibrationSessionControlViewModel& calibration_vm_;
        CalibrationResultSaveViewModel& calibration_result_save_vm_;
    };

}


#endif //CLEANGRADUATOR_CONTROLVIEWMODEL_H