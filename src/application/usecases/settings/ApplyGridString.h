#ifndef CLEANGRADUATOR_APPLYCAMERAGRIDSETTINGS_H
#define CLEANGRADUATOR_APPLYCAMERAGRIDSETTINGS_H
#include "application/dto/settings/grid/VideoSourceGridSettings.h"
#include "application/dto/settings/grid/VideoSourceGridString.h"
#include "application/ports/outbound/video/IVideoSourceCrosshairListener.h"
#include "application/services/VideoSourceGridServiceResult.h"
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
        services::VideoSourceGridService& grid_service_;
    };
}

#endif //CLEANGRADUATOR_APPLYCAMERAGRIDSETTINGS_H