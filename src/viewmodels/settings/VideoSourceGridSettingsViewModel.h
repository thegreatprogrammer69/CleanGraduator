#ifndef CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_SETTINGSVIEWMODEL_H
#include "application/dto/settings/camera_grid/VideoSourceGridSettings.h"
#include "viewmodels/Observable.h"


namespace application::usecases {
    class ApplyCameraGridSettings;
}

namespace mvvm {

    class VideoSourceGridSettingsViewModel {
    public:
        Observable<application::dto::VideoSourceGridSettings> settings;
        Observable<std::string> error;

    public:
        explicit VideoSourceGridSettingsViewModel(application::usecases::ApplyCameraGridSettings& use_case);
        ~VideoSourceGridSettingsViewModel();

        void applySettings();

    private:
        application::usecases::ApplyCameraGridSettings& use_case_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSVIEWMODEL_H