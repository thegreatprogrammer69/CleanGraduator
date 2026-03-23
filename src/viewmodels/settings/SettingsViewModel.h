#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H

namespace mvvm {
    class CameraGridSettingsViewModel;
    class CircleOverlaySettingsViewModel;
    class InfoSettingsViewModel;

    struct SettingsViewModelDeps {
        CameraGridSettingsViewModel& camera_grid;
        CircleOverlaySettingsViewModel& circle_overlay_settings;
        InfoSettingsViewModel& info_settings;
    };

    class SettingsViewModel {
    public:
        explicit SettingsViewModel(SettingsViewModelDeps deps);

        ~SettingsViewModel();

        CameraGridSettingsViewModel& cameraGridViewModel();
        CircleOverlaySettingsViewModel& circleOverlaySettingsViewModel();
        InfoSettingsViewModel& infoSettingsViewModel();

    private:
        CameraGridSettingsViewModel& camera_grid_;
        CircleOverlaySettingsViewModel& circle_overlay_settings_;
        InfoSettingsViewModel& info_settings_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H
