#include "ControlViewModel.h"

mvvm::ControlViewModel::ControlViewModel(ControlViewModelDeps deps): motor_vm_(deps.motor_vm)
                                                                     , valves_vm_(deps.valves_vm) {}

mvvm::MotorControlViewModel & mvvm::ControlViewModel::motorViewModel() { return motor_vm_; }

mvvm::DualValveControlViewModel & mvvm::ControlViewModel::valvesViewModel() { return valves_vm_; }
