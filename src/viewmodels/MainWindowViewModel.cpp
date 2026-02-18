#include "MainWindowViewModel.h"


mvvm::MainWindowViewModel::MainWindowViewModel(MainWindowViewModelDeps deps)
    : deps_(deps)
{
}

mvvm::MainWindowViewModel::~MainWindowViewModel() {
}

mvvm::LogViewerViewModel & mvvm::MainWindowViewModel::logViewerViewModel() {
    return deps_.log_viewer;
}

mvvm::VideoSourceGridViewModel & mvvm::MainWindowViewModel::videoSourceGridViewModel() {
    return deps_.grid;
}

mvvm::SettingsViewModel & mvvm::MainWindowViewModel::settingsViewModel() {
    return deps_.settings;
}

mvvm::ControlViewModel & mvvm::MainWindowViewModel::controlViewModel() {
    return deps_.control;
}

mvvm::StatusBarViewModels mvvm::MainWindowViewModel::statusBarViewModels() {
    return deps_.status_bar;
}


