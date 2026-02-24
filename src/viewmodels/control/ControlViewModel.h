#ifndef CLEANGRADUATOR_CONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CONTROLVIEWMODEL_H


namespace mvvm {
    class DualValveControlViewModel;
    class MotorControlViewModel;
    class CalibrationSessionControlViewModel;

    struct ControlViewModelDeps {
        DualValveControlViewModel& valves_vm;
        MotorControlViewModel& motor_vm;
        CalibrationSessionControlViewModel& calibration_vm;
    };

    class ControlViewModel {
    public:
        explicit ControlViewModel(ControlViewModelDeps deps);
        MotorControlViewModel& motorViewModel();
        DualValveControlViewModel& valvesViewModel();
        CalibrationSessionControlViewModel& calibrationViewModel();

    private:
        MotorControlViewModel& motor_vm_;
        DualValveControlViewModel& valves_vm_;
        CalibrationSessionControlViewModel& calibration_vm_;
    };

}


#endif //CLEANGRADUATOR_CONTROLVIEWMODEL_H