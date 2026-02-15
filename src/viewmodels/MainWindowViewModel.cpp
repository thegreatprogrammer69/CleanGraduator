#include "MainWindowViewModel.h"


mvvm::MainWindowViewModel::MainWindowViewModel(MainWindowViewModelDeps deps)
    : deps_(deps)
{
}

mvvm::MainWindowViewModel::~MainWindowViewModel() {
}

mvvm::VideoSourceGridViewModel & mvvm::MainWindowViewModel::videoSourceGridViewModel() {
    return deps_.grid;
}

mvvm::SettingsViewModel & mvvm::MainWindowViewModel::settingsViewModel() {
    return deps_.settings;
}


