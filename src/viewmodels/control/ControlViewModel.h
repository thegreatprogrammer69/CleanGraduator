#ifndef CLEANGRADUATOR_CONTROLVIEWMODEL_H
#define CLEANGRADUATOR_CONTROLVIEWMODEL_H


namespace mvvm {
    class DualValveControlViewModel;
    class MotorControlViewModel;

    struct ControlViewModelDeps {
        DualValveControlViewModel& valves_vm;
        MotorControlViewModel& motor_vm;
    };

    class ControlViewModel {
    public:
        explicit ControlViewModel(ControlViewModelDeps deps);
        MotorControlViewModel& motorViewModel();
        DualValveControlViewModel& valvesViewModel();

    private:
        MotorControlViewModel& motor_vm_;
        DualValveControlViewModel& valves_vm_;
    };

}


#endif //CLEANGRADUATOR_CONTROLVIEWMODEL_H