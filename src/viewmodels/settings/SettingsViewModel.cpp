#include "SettingsViewModel.h"

mvvm::SettingsViewModel::SettingsViewModel(SettingsViewModelDeps deps)
    : camera_grid_(deps.camera_grid) {}

mvvm::SettingsViewModel::~SettingsViewModel() {
}


mvvm::CameraGridSettingsViewModel & mvvm::SettingsViewModel::cameraGridViewModel() {
    return camera_grid_;
}

