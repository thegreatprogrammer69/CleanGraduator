#include "SettingsViewModel.h"

#include "CircleOverlaySettingsViewModel.h"

mvvm::SettingsViewModel::SettingsViewModel(SettingsViewModelDeps deps)
    : camera_grid_(deps.camera_grid)
    , circle_overlay_settings_(deps.circle_overlay_settings)
    , info_settings_(deps.info_settings) {}

mvvm::SettingsViewModel::~SettingsViewModel() {
}

mvvm::CameraGridSettingsViewModel & mvvm::SettingsViewModel::cameraGridViewModel() {
    return camera_grid_;
}

mvvm::CircleOverlaySettingsViewModel& mvvm::SettingsViewModel::circleOverlaySettingsViewModel() {
    return circle_overlay_settings_;
}

mvvm::InfoSettingsViewModel& mvvm::SettingsViewModel::infoSettingsViewModel() {
    return info_settings_;
}
