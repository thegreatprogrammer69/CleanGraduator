#ifndef CLEANGRADUATOR_APPLYVIDEOGRIDSETTINGS_H
#define CLEANGRADUATOR_APPLYVIDEOGRIDSETTINGS_H
#include "application/settings/video/VideoGridSettings.h"
#include "domain/fmt/FmtLogger.h"

namespace application::services {
    class VideoSourceRepository;
}

namespace application::ports {
    struct ISettingsStorage;
}

namespace domain::ports {
    struct ILogger;
}

namespace application::usecase {
    struct ApplyVideoGridSettingsResult {
        models::VideoGridSettings corrected_settings;
    };

    struct ApplyVideoGridSettingsPorts {
        domain::ports::ILogger& logger;
        ports::ISettingsStorage& settings_storage;
        services::VideoSourceRepository& video_source_repository;
    };

    class ApplyVideoGridSettings {
    public:
        explicit ApplyVideoGridSettings(ApplyVideoGridSettingsPorts ports);
        ~ApplyVideoGridSettings();
        ApplyVideoGridSettingsResult execute(const models::VideoGridSettings& settings);
    private:
        fmt::FmtLogger logger_;
        ports::ISettingsStorage& settings_storage_;
        services::VideoSourceRepository& video_source_repository_;
    };
}

#endif //CLEANGRADUATOR_APPLYVIDEOGRIDSETTINGS_H