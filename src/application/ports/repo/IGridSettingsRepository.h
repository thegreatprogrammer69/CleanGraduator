#ifndef CLEANGRADUATOR_IGRIDSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_IGRIDSETTINGSREPOSITORY_H
#include "application/dto/settings/grid/VideoSourceGridSettings.h"

namespace application::ports {
    class IGridSettingsRepository {
    public:
        virtual void save(const dto::VideoSourceGridSettings&) = 0;
        virtual dto::VideoSourceGridSettings load() const = 0;
        virtual ~IGridSettingsRepository() = default;
    };
}

#endif //CLEANGRADUATOR_IGRIDSETTINGSREPOSITORY_H