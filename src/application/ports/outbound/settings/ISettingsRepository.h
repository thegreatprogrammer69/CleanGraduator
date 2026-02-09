#ifndef CLEANGRADUATOR_IUSERSETTINGSREPOSITORY_H
#define CLEANGRADUATOR_IUSERSETTINGSREPOSITORY_H
#include <optional>

#include "application/dto/UserSettings.h"
#include "application/model/CameraString.h"
#include "../../../dto/PrinterRecord.h"
#include "application/model/DisplacementRecord.h"
#include "application/model/GaugeRecord.h"
#include "application/model/PrecisionRecord.h"
#include "application/model/PressureUnitRecord.h"

namespace application::ports {

    struct ISettingsRepository {
        virtual ~ISettingsRepository() = default;
        virtual dto::UserSettings load() const = 0;
        virtual void save(const dto::UserSettings& settings) = 0;
    };

} // namespace application::ports

#endif //CLEANGRADUATOR_IUSERSETTINGSREPOSITORY_H