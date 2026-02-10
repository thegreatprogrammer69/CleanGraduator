#ifndef CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H

#include <string>

#include "application/dto/UserSettings.h"
#include "application/ports/outbound/settings/IVideoSourceGridSettingsRepository.h"

namespace infrastructure::settings {

    class QtCameraGridSettingsRepository final : public application::ports::IVideoSourceGridSettingsRepository {
    public:
        explicit QtCameraGridSettingsRepository(std::string iniFilePath);

        application::dto::UserSettings load() const override;
        void save(const application::dto::UserSettings& settings) override;

    private:
        std::string path_;
    };

} // namespace infrastructure::settings

#endif //CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H