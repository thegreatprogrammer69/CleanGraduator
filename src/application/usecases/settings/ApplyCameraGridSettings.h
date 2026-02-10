#ifndef CLEANGRADUATOR_APPLYCAMERAGRIDSETTINGS_H
#define CLEANGRADUATOR_APPLYCAMERAGRIDSETTINGS_H
#include "application/dto/settings/camera_grid/VideoSourceGridSettings.h"
#include "application/dto/settings/camera_grid/VideoSourceGridString.h"
#include "application/ports/outbound/video/IVideoSourceCrosshairListener.h"
#include "application/services/VideoSourceGridServiceApplyResult.h"
#include "domain/fmt/FmtLogger.h"

namespace application::services {
    class VideoSourceGridService;
}

namespace domain::ports {
    struct ILogger;
}

namespace application::dto {
    struct VideoSourceGridSettings;
}

namespace application::ports {
    struct IVideoSourceGridSettingsRepository;
}

namespace application::usecases {
    struct ApplyCameraGridSettingsPorts {
        domain::ports::ILogger& logger;
        ports::IVideoSourceGridSettingsRepository& settings_repo;
        ports::IVideoSourceCrosshairListener& crosshair_listener;
    };

    struct ApplyCameraGridSettingsServices {
        services::VideoSourceGridService& grid_service;
    };

    class ApplyCameraGridSettings {
    public:
        explicit ApplyCameraGridSettings(ApplyCameraGridSettingsPorts ports, ApplyCameraGridSettingsServices services);
        ~ApplyCameraGridSettings();

        struct ExecuteResult {
            std::string error;
            dto::VideoSourceGridSettings correct_settings;
        };
        ExecuteResult execute(const dto::VideoSourceGridSettings& settings);

    private:
        fmt::FmtLogger logger_;
        ports::IVideoSourceGridSettingsRepository& settings_repo_;
        ports::IVideoSourceCrosshairListener& crosshair_listener_;
        services::VideoSourceGridService& grid_service_;
    };
}

#endif //CLEANGRADUATOR_APPLYCAMERAGRIDSETTINGS_H