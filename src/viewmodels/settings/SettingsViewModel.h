#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H

namespace mvvm {
    class CameraGridSettingsViewModel;

    struct SettingsViewModelDeps {
        CameraGridSettingsViewModel& camera_grid;
    };

    class SettingsViewModel {
    public:
        explicit SettingsViewModel(SettingsViewModelDeps deps);

        ~SettingsViewModel();

        CameraGridSettingsViewModel& cameraGridViewModel();

    private:
        CameraGridSettingsViewModel& camera_grid_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H