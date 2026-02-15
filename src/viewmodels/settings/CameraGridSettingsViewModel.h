#ifndef CLEANGRADUATOR_CAMERAGRIDSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_CAMERAGRIDSETTINGSVIEWMODEL_H
#include <string>

#include "viewmodels/Observable.h"

namespace application::orchestrators {
    class VideoSourceManager;
}

namespace application::usecase {
    class CloseAllCameras;
    class OpenAllCameras;
    class OpenSelectedCameras;
}

namespace mvvm {
    struct CameraGridSettingsViewModelDeps {
        application::usecase::OpenSelectedCameras& open_selected;
        application::usecase::OpenAllCameras& open_all;
        application::usecase::CloseAllCameras& close_all;
    };

    class CameraGridSettingsViewModel {
    public:
        CameraGridSettingsViewModel(CameraGridSettingsViewModelDeps deps);

        Observable<std::string> cameraInput;

        void open();
        void openAll();
        void closeAll();

    private:
        void setIndexes(const std::vector<int> &indexes);

    private:
        CameraGridSettingsViewModelDeps deps_;
    };
}


#endif //CLEANGRADUATOR_CAMERAGRIDSETTINGSVIEWMODEL_H