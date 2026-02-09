#ifndef CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H

#include <string>
#include <optional>

#include "application/dto/UserSettings.h"
#include "application/ports/outbound/settings/IUserSettingsRepository.h"

namespace infrastructure::settings {

    class FileUserSettingsRepository final : public application::ports::IUserSettingsRepository {
    public:
        explicit FileUserSettingsRepository(std::string iniFilePath);

        application::dto::UserSettings load() const override;
        void save(const application::dto::UserSettings& settings) override;

    private:
        std::string path_;
    };

} // namespace infrastructure::settings

#endif //CLEANGRADUATOR_FILEUSERSETTINGSREPOSITORY_H