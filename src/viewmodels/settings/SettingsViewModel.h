#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H

namespace mvvm {
    class CameraGridSettingsViewModel;
    class InfoSettingsViewModel;

    struct SettingsViewModelDeps {
        CameraGridSettingsViewModel& camera_grid;
        InfoSettingsViewModel& info_settings;
    };

    class SettingsViewModel {
    public:
        explicit SettingsViewModel(SettingsViewModelDeps deps);

        ~SettingsViewModel();

        CameraGridSettingsViewModel& cameraGridViewModel();
        InfoSettingsViewModel& infoSettingsViewModel();

    private:
        CameraGridSettingsViewModel& camera_grid_;
        InfoSettingsViewModel& info_settings_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H
