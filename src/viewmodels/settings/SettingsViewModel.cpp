#include "SettingsViewModel.h"

mvvm::SettingsViewModel::SettingsViewModel(SettingsViewModelDeps deps)
    : camera_grid_(deps.camera_grid)
    , info_settings_(deps.info_settings)
    , circle_overlay_(deps.circle_overlay) {}

mvvm::SettingsViewModel::~SettingsViewModel() {
}

mvvm::CameraGridSettingsViewModel & mvvm::SettingsViewModel::cameraGridViewModel() {
    return camera_grid_;
}

mvvm::InfoSettingsViewModel& mvvm::SettingsViewModel::infoSettingsViewModel() {
    return info_settings_;
}

mvvm::CircleOverlaySettingsViewModel& mvvm::SettingsViewModel::circleOverlayViewModel() {
    return circle_overlay_;
}
