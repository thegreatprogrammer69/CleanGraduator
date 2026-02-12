#ifndef CLEANGRADUATOR_ICOMPONENTSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_ICOMPONENTSETTINGSREPOSITORY_H
#include "application/models/config_parameter/ComponentSettings.h"

namespace application::ports {

    class IComponentSettingsRepository {
    public:
        virtual ~IComponentSettingsRepository() = default;

        virtual void save(const models::ComponentSettings& settings) = 0;

        virtual models::ComponentSettings load() const = 0;
    };

}

#endif //CLEANGRADUATOR_ICOMPONENTSETTINGSREPOSITORY_H