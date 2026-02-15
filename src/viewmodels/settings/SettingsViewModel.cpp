#include "SettingsViewModel.h"

mvvm::SettingsViewModel::SettingsViewModel(SettingsViewModelDeps deps)
    : camera_grid_(deps.camera_grid)
    , info_settings_(deps.info_settings) {}

mvvm::SettingsViewModel::~SettingsViewModel() {
}

mvvm::CameraGridSettingsViewModel & mvvm::SettingsViewModel::cameraGridViewModel() {
    return camera_grid_;
}

mvvm::InfoSettingsViewModel& mvvm::SettingsViewModel::infoSettingsViewModel() {
    return info_settings_;
}
