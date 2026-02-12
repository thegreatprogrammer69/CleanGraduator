#ifndef CLEANGRADUATOR_CAMERAGRIDSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_CAMERAGRIDSETTINGSVIEWMODEL_H
#include <string>

#include "viewmodels/Observable.h"

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::usecase {
    class OpenSelectedCameras;
}

namespace mvvm {
    class CameraGridSettingsViewModel {
    public:
        CameraGridSettingsViewModel(application::usecase::OpenSelectedCameras& open_use_sase,
            application::orchestrators::VideoSourceManager& manager);

        Observable<std::string> cameraInput;

        void open();
        void openAll();
        void closeAll();

    private:
        application::usecase::OpenSelectedCameras& open_use_sase_;
        application::orchestrators::VideoSourceManager& manager_;
    };
}


#endif //CLEANGRADUATOR_CAMERAGRIDSETTINGSVIEWMODEL_H