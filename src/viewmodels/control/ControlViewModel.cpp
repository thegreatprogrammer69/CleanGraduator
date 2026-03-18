#include "ControlViewModel.h"

mvvm::ControlViewModel::ControlViewModel(ControlViewModelDeps deps): motor_vm_(deps.motor_vm)
                                                                     , valves_vm_(deps.valves_vm)
                                                                     , calibration_vm_(deps.calibration_vm)
                                                                     , calibration_result_save_vm_(deps.calibration_result_save_vm) {}

mvvm::MotorControlViewModel & mvvm::ControlViewModel::motorViewModel() { return motor_vm_; }

mvvm::DualValveControlViewModel & mvvm::ControlViewModel::valvesViewModel() { return valves_vm_; }


mvvm::CalibrationSessionControlViewModel & mvvm::ControlViewModel::calibrationViewModel() { return calibration_vm_; }


mvvm::CalibrationResultSaveViewModel & mvvm::ControlViewModel::calibrationResultSaveViewModel() { return calibration_result_save_vm_; }
