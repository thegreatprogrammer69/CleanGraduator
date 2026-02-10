#ifndef CLEANGRADUATOR_ICameraGridSettingsRepository_H
#define CLEANGRADUATOR_ICameraGridSettingsRepository_H
#include "application/dto/UserSettings.h"

namespace application::ports {

    struct ICameraGridSettingsRepository {
        virtual ~ICameraGridSettingsRepository() = default;
        virtual dto::UserSettings load() const = 0;
        virtual void save(const dto::UserSettings& settings) = 0;
    };

} // namespace application::ports

#endif //CLEANGRADUATOR_ICameraGridSettingsRepository_H