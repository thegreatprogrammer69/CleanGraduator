#ifndef CLEANGRADUATOR_CATALOGSASSEMBLER_H
#define CLEANGRADUATOR_CATALOGSASSEMBLER_H

#include <memory>
#include <string>

namespace domain::ports { struct ILogger; }
namespace application::ports {
    struct IDisplacementCatalog;
    struct IGaugeCatalog;
    struct IPrecisionCatalog;
    struct IPressureUnitCatalog;
    struct IPrinterCatalog;
    struct IInfoSettingsStorage;
}

namespace app::bootstrap {

using LoggerFactoryFn = domain::ports::ILogger&(*)(void*, const std::string&);

struct CatalogsAssembly {
    std::unique_ptr<application::ports::IDisplacementCatalog> displacement_catalog;
    std::unique_ptr<application::ports::IGaugeCatalog> gauge_catalog;
    std::unique_ptr<application::ports::IPrecisionCatalog> precision_catalog;
    std::unique_ptr<application::ports::IPressureUnitCatalog> pressure_unit_catalog;
    std::unique_ptr<application::ports::IPrinterCatalog> printer_catalog;
    std::unique_ptr<application::ports::IInfoSettingsStorage> info_settings_storage;
};

CatalogsAssembly assembleCatalogs(const std::string& catalogs_dir, LoggerFactoryFn logger_factory, void* logger_context);

}

#endif //CLEANGRADUATOR_CATALOGSASSEMBLER_H
