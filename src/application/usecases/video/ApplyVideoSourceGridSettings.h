#ifndef CLEANGRADUATOR_APPLYVIDEOSOURCEGRIDSETTINGS_H
#define CLEANGRADUATOR_APPLYVIDEOSOURCEGRIDSETTINGS_H
#include "ApplyVideoSourceGridSettingsResult.h"
#include "domain/fmt/FmtLogger.h"

namespace domain::ports {
    struct ILogger;
}

namespace domain::events {
    class IEventBus;
}

namespace application::usecase {
    struct ApplyVideoSourceGridSettingsPorts {
        domain::ports::ILogger& logger;
        domain::events::IEventBus& event_bus;
    };

    class ApplyVideoSourceGridSettings final {
    public:
        explicit ApplyVideoSourceGridSettings(ApplyVideoSourceGridSettingsPorts ports);
        ~ApplyVideoSourceGridSettings() = default;
        ApplyVideoSourceGridSettingsResult execute(const dto::VideoSourceGridSettings& input);

    private:
        fmt::FmtLogger logger_;
        domain::events::IEventBus& event_bus_;
    };
}

#endif //CLEANGRADUATOR_APPLYVIDEOSOURCEGRIDSETTINGS_H