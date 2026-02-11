#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#include "application/dto/settings/grid/VideoSourceGridSettings.h"
#include "viewmodels/Observable.h"


namespace application::usecases {
    class ApplyCameraGridSettings;
}

namespace mvvm {

    class VideoSourceGridSettingsViewModel {
    public:
        Observable<application::dto::VideoSourceCrosshair> crosshair{};
        Observable<application::dto::VideoSourceGridString> string{};
        Observable<bool> open_cameras_at_startup{};
        Observable<std::string> error{};

    public:
        explicit VideoSourceGridSettingsViewModel(
            application::dto::VideoSourceGridSettings settings,
            application::usecases::ApplyCameraGridSettings& use_case
        );
        ~VideoSourceGridSettingsViewModel();

        void applySettings();

    private:
        application::usecases::ApplyCameraGridSettings& use_case_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H