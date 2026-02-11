#ifndef CLEANGRADUATOR_OPENALLCAMERAS_H
#define CLEANGRADUATOR_OPENALLCAMERAS_H

namespace application::services {
    class VideoSourceGridService;
}

namespace application::ports {
    struct IVideoSourceGridSettingsRepository;
}

namespace domain::ports {
    struct ILogger;
}

struct ApplyCameraGridSettingsPorts {
    domain::ports::ILogger& logger;
};

struct ApplyCameraGridSettingsServices {
    application::services::VideoSourceGridService& grid_service;
};

namespace application::usecases {
    class OpenAllCameras {

    };
}

#endif //CLEANGRADUATOR_OPENALLCAMERAS_H