#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H

namespace mvvm {
    class CameraGridSettingsViewModel;
    class InfoSettingsViewModel;
    class CircleOverlaySettingsViewModel;

    struct SettingsViewModelDeps {
        CameraGridSettingsViewModel& camera_grid;
        InfoSettingsViewModel& info_settings;
        CircleOverlaySettingsViewModel& circle_overlay;
    };

    class SettingsViewModel {
    public:
        explicit SettingsViewModel(SettingsViewModelDeps deps);

        ~SettingsViewModel();

        CameraGridSettingsViewModel& cameraGridViewModel();
        InfoSettingsViewModel& infoSettingsViewModel();
        CircleOverlaySettingsViewModel& circleOverlayViewModel();

    private:
        CameraGridSettingsViewModel& camera_grid_;
        InfoSettingsViewModel& info_settings_;
        CircleOverlaySettingsViewModel& circle_overlay_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H
