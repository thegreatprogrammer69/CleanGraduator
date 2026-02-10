#include "MainWindowViewModel.h"

#include "settings/VideoSourceGridSettingsViewModel.h"

mvvm::MainWindowViewModel::MainWindowViewModel(
    VideoSourceGridViewModel &video_source_grid,
    VideoSourceGridSettingsViewModel &video_source_grid_settings)

    : video_source_grid_(video_source_grid)
    , video_source_grid_settings_(video_source_grid_settings)
{
}

mvvm::VideoSourceGridViewModel & mvvm::MainWindowViewModel::videoSourceGridModel() {
    return video_source_grid_;
}

mvvm::VideoSourceGridSettingsViewModel & mvvm::MainWindowViewModel::videoSourceGridSettingsViewModel() {
    return video_source_grid_settings_;
}
