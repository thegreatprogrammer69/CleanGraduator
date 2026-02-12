#include "MainWindowViewModel.h"


mvvm::MainWindowViewModel::MainWindowViewModel(
    VideoSourceGridViewModel &video_source_grid,
    CameraGridSettingsViewModel &camera_grid_settings)

    : video_source_grid_(video_source_grid)
    , camera_grid_settings_(camera_grid_settings)
{
}

mvvm::VideoSourceGridViewModel & mvvm::MainWindowViewModel::videoSourceGridModel() {
    return video_source_grid_;
}

mvvm::CameraGridSettingsViewModel & mvvm::MainWindowViewModel::cameraGridSettingsViewModel() {
    return camera_grid_settings_;
}

