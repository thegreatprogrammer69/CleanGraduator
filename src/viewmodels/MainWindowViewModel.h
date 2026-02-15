#ifndef CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
#define CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H


namespace mvvm {
    class SettingsViewModel;
    class VideoSourceGridViewModel;

    struct MainWindowViewModelDeps {
        VideoSourceGridViewModel &grid;
        SettingsViewModel &settings;
    };

    class MainWindowViewModel {
    public:
        explicit MainWindowViewModel(MainWindowViewModelDeps deps);
        ~MainWindowViewModel();
        VideoSourceGridViewModel& videoSourceGridViewModel();
        SettingsViewModel& settingsViewModel();

    private:
        MainWindowViewModelDeps deps_;
    };
}


#endif //CLEANGRADUATOR_MAINWINDOWVIEWMODEL_H
