#ifndef CLEANGRADUATOR_QTINFOSETTINGSSTORAGE_H
#define CLEANGRADUATOR_QTINFOSETTINGSSTORAGE_H

#include <QSettings>

#include "application/ports/catalogs/IDisplacementCatalog.h"
#include "application/ports/catalogs/IGaugeCatalog.h"
#include "application/ports/catalogs/IGaugePrecisionCatalog.h"
#include "application/ports/catalogs/IPressureUnitCatalog.h"
#include "application/ports/catalogs/IPrinterCatalog.h"
#include "application/ports/settings/IInfoSettingsStorage.h"

namespace infra::storage {
    struct QtInfoSettingsStorageCatalogs {
        application::ports::IDisplacementCatalog& displacement_catalog;
        application::ports::IGaugeCatalog& gauge_catalog;
        application::ports::IGaugePrecisionCatalog& precision_catalog;
        application::ports::IPressureUnitCatalog& pressure_unit_catalog;
        application::ports::IPrinterCatalog& printer_catalog;
    };

    class QtInfoSettingsStorage final : public application::ports::IInfoSettingsStorage {
    public:
        QtInfoSettingsStorage(
            QString organization,
            QString application,
            QtInfoSettingsStorageCatalogs catalogs
        );

        application::ports::InfoSettingsData loadInfoSettings() override;
        void saveInfoSettings(const application::ports::InfoSettingsData& data) override;

        ~QtInfoSettingsStorage() override;

    private:
        int clampToCatalog(int idx, int catalog_size) const;

    private:
        QSettings settings_;
        QtInfoSettingsStorageCatalogs catalogs_;
    };
}

#endif //CLEANGRADUATOR_QTINFOSETTINGSSTORAGE_H
