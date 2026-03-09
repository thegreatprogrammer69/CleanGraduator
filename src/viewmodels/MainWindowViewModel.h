#ifndef CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#define CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#include "status_bar/StatusBarViewModels.h"


namespace mvvm {
    class ControlViewModel;
    class LogViewerViewModel;
    class SettingsViewModel;
    class VideoSourceGridViewModel;
    class CalibrationSeriesViewModel;

    struct MainWindowViewModelDeps {
        CalibrationSeriesViewModel &calibration_series;
        LogViewerViewModel &log_viewer;
        VideoSourceGridViewModel &grid;
        SettingsViewModel &settings;
        ControlViewModel &control;
        StatusBarViewModels status_bar;
    };

    class MainWindowViewModel {
    public:
        explicit MainWindowViewModel(MainWindowViewModelDeps deps);
        ~MainWindowViewModel();
        CalibrationSeriesViewModel& calibrationSeries();
        LogViewerViewModel& logViewerViewModel();
        VideoSourceGridViewModel& videoSourceGridViewModel();
        SettingsViewModel& settingsViewModel();
        ControlViewModel& controlViewModel();
        StatusBarViewModels statusBarViewModels();

    private:
        MainWindowViewModelDeps deps_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
