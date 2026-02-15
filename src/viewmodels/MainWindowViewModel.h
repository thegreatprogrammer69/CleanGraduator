#ifndef CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#define CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H


namespace mvvm {
    class LogViewerViewModel;
    class SettingsViewModel;
    class VideoSourceGridViewModel;

    struct MainWindowViewModelDeps {
        LogViewerViewModel &log_viewer;
        VideoSourceGridViewModel &grid;
        SettingsViewModel &settings;
    };

    class MainWindowViewModel {
    public:
        explicit MainWindowViewModel(MainWindowViewModelDeps deps);
        ~MainWindowViewModel();
        LogViewerViewModel& logViewerViewModel();
        VideoSourceGridViewModel& videoSourceGridViewModel();
        SettingsViewModel& settingsViewModel();

    private:
        MainWindowViewModelDeps deps_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
