#ifndef CLEANGRADUATOR_INFOSETTINGSSERVICE_H
#define CLEANGRADUATOR_INFOSETTINGSSERVICE_H
#include <filesystem>
#include <optional>

#include "application/models/info/Gauge.h"
#include "application/models/info/GaugePrecision.h"
#include "application/models/info/PressureUnit.h"
#include "application/models/info/Printer.h"
#include "application/ports/settings/IInfoSettingsStorage.h"
#include "../../../domain/core/calibration/common/PressurePoints.h"

namespace application::models {
    struct Displacement;
}

namespace application::ports {
    struct IPrinterCatalog;
    struct IPressureUnitCatalog;
    struct IGaugePrecisionCatalog;
    struct IGaugeCatalog;
    struct IDisplacementCatalog;
    struct IInfoSettingsStorage;
}

namespace application::orchestrators {
    struct CalibrationSettingsQueryPorts {
        ports::IInfoSettingsStorage& storage;

        ports::IDisplacementCatalog& displacement_catalog;
        ports::IGaugeCatalog& gauge_catalog;
        ports::IGaugePrecisionCatalog& precision_catalog;
        ports::IPressureUnitCatalog& pressure_unit_catalog;
        ports::IPrinterCatalog& printer_catalog;
    };

    class CalibrationSettingsQuery {
    public:
        explicit CalibrationSettingsQuery(CalibrationSettingsQueryPorts ports);

        std::optional<domain::common::PressurePoints> currentGaugePressurePoints() const;
        std::optional<domain::common::GaugePrecision> currentPrecision() const;
        std::optional<domain::common::PressureUnit> currentPressureUnit() const;

        std::optional<std::filesystem::path> currentSaveResultPath() const;


    private:
        void load();
        void normalize();

    private:
        ports::IInfoSettingsStorage& storage_;

        ports::IDisplacementCatalog& displacement_catalog_;
        ports::IGaugeCatalog& gauge_catalog_;
        ports::IGaugePrecisionCatalog& precision_catalog_;
        ports::IPressureUnitCatalog& pressure_unit_catalog_;
        ports::IPrinterCatalog& printer_catalog_;

        ports::InfoSettingsData data_;
    };
}


#endif //CLEANGRADUATOR_INFOSETTINGSSERVICE_H