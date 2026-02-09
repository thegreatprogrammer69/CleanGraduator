#include "MainWindowViewModel.h"

mvvm::MainWindowViewModel::MainWindowViewModel(VideoSourceGridViewModel &video_source_grid)
    : video_source_grid_(video_source_grid)
{
}

mvvm::VideoSourceGridViewModel & mvvm::MainWindowViewModel::videoSourceGridModel() {
    return video_source_grid_;
}
