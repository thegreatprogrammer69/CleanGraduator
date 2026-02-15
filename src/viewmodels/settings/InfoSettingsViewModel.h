#ifndef CLEANGRADUATOR_INFOSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_INFOSETTINGSVIEWMODEL_H

#include <string>
#include <vector>

#include "application/ports/outbound/catalogs/IDisplacementCatalog.h"
#include "application/ports/outbound/catalogs/IGaugeCatalog.h"
#include "application/ports/outbound/catalogs/IPrecisionCatalog.h"
#include "application/ports/outbound/catalogs/IPressureUnitCatalog.h"
#include "application/ports/outbound/catalogs/IPrinterCatalog.h"
#include "application/ports/outbound/settings/IInfoSettingsStorage.h"
#include "viewmodels/Observable.h"

namespace mvvm {
    struct InfoSettingsViewModelDeps {
        application::ports::IInfoSettingsStorage& storage;
        application::ports::IDisplacementCatalog& displacement_catalog;
        application::ports::IGaugeCatalog& gauge_catalog;
        application::ports::IPrecisionCatalog& precision_catalog;
        application::ports::IPressureUnitCatalog& pressure_unit_catalog;
        application::ports::IPrinterCatalog& printer_catalog;
    };

    class InfoSettingsViewModel {
    public:
        explicit InfoSettingsViewModel(InfoSettingsViewModelDeps deps);

        const std::vector<std::string>& displacements() const;
        const std::vector<std::string>& gauges() const;
        const std::vector<std::string>& precisions() const;
        const std::vector<std::string>& pressureUnits() const;
        const std::vector<std::string>& printers() const;

        Observable<int> selectedDisplacement;
        Observable<int> selectedGauge;
        Observable<int> selectedPrecision;
        Observable<int> selectedPressureUnit;
        Observable<int> selectedPrinter;

        void save();

    private:
        void loadCatalogs();
        void loadSelection();
        static std::string toUtf8(const std::wstring& value);

    private:
        InfoSettingsViewModelDeps deps_;

        std::vector<std::string> displacements_;
        std::vector<std::string> gauges_;
        std::vector<std::string> precisions_;
        std::vector<std::string> pressure_units_;
        std::vector<std::string> printers_;
    };
}

#endif //CLEANGRADUATOR_INFOSETTINGSVIEWMODEL_H
