#include "MainWindowViewModel.h"


mvvm::MainWindowViewModel::MainWindowViewModel(MainWindowViewModelDeps deps)
    : deps_(deps)
{
}

mvvm::MainWindowViewModel::~MainWindowViewModel() {
}

mvvm::CalibrationSeriesViewModel & mvvm::MainWindowViewModel::calibrationSeries() {
    return deps_.calibration_series;
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

mvvm::CalibrationResultTableViewModel & mvvm::MainWindowViewModel::calibrationResultTableViewModel() {
    return deps_.calibration_result_table;
}

mvvm::CalibrationResultSaveViewModel& mvvm::MainWindowViewModel::calibrationResultSaveViewModel() {
    return deps_.calibration_result_save;
}

mvvm::StatusBarViewModels mvvm::MainWindowViewModel::statusBarViewModels() {
    return deps_.status_bar;
}

