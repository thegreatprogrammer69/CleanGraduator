#ifndef CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H

#include <string>
#include <optional>

#include "application/dto/settings/grid/VideoSourceGridSettings.h"
#include "application/ports/outbound/settings/IVideoSourceGridSettingsRepository.h"

namespace infrastructure::settings {

    class QtCameraGridSettingsRepository final
        : public application::ports::IVideoSourceGridSettingsRepository {
    public:
        explicit QtCameraGridSettingsRepository(std::string iniFilePath);

        application::dto::VideoSourceGridSettings loadGridSettings() const override;
        void saveGridSettings(const application::dto::VideoSourceGridSettings& settings) override;

    private:
        application::dto::VideoSourceGridSettings loadFromFile() const;

    private:
        std::string path_;

        // ---- Cache ----
        mutable std::optional<application::dto::VideoSourceGridSettings> cache_;
    };

} // namespace infrastructure::settings

#endif
