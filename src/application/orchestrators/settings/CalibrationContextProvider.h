#ifndef CLEANGRADUATOR_CALIBRATIONCONTEXTPROVIDER_H
#define CLEANGRADUATOR_CALIBRATIONCONTEXTPROVIDER_H

#include <optional>

#include "application/models/info/CalibrationContext.h"
#include "application/ports/settings/IInfoSettingsStorage.h"
#include "domain/core/calibration/common/PressurePoints.h"

namespace application::ports {
    struct IPrinterCatalog;
    struct IPressureUnitCatalog;
    struct IGaugePrecisionCatalog;
    struct IGaugeCatalog;
    struct IDisplacementCatalog;
}

namespace application::orchestrators {
    struct CalibrationContextProviderPorts {
        ports::IInfoSettingsStorage& storage;
        ports::IDisplacementCatalog& displacement_catalog;
        ports::IGaugeCatalog& gauge_catalog;
        ports::IGaugePrecisionCatalog& precision_catalog;
        ports::IPressureUnitCatalog& pressure_unit_catalog;
        ports::IPrinterCatalog& printer_catalog;
    };

    class CalibrationContextProvider {
    public:
        explicit CalibrationContextProvider(CalibrationContextProviderPorts ports);

        std::optional<models::CalibrationContext> current() const;
        std::optional<domain::common::PressurePoints> currentGaugePressurePoints() const;

    private:
        void normalize(ports::InfoSettingsData& data) const;

    private:
        ports::IInfoSettingsStorage& storage_;
        ports::IDisplacementCatalog& displacement_catalog_;
        ports::IGaugeCatalog& gauge_catalog_;
        ports::IGaugePrecisionCatalog& precision_catalog_;
        ports::IPressureUnitCatalog& pressure_unit_catalog_;
        ports::IPrinterCatalog& printer_catalog_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONCONTEXTPROVIDER_H
