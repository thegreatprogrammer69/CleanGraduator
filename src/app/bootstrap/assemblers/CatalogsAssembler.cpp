#include "CatalogsAssembler.h"

#include "infrastructure/catalogs/FileDisplacementCatalog.h"
#include "infrastructure/catalogs/FileGaugeCatalog.h"
#include "infrastructure/catalogs/FilePrecisionCatalog.h"
#include "infrastructure/catalogs/FilePressureUnitCatalog.h"
#include "infrastructure/catalogs/FilePrinterCatalog.h"
#include "infrastructure/storage/QtInfoSettingsStorage.h"

using namespace app::bootstrap;

CatalogsAssembly app::bootstrap::assembleCatalogs(const std::string &catalogs_dir, LoggerFactoryFn logger_factory, void *logger_context) {
    CatalogsAssembly assembled;

    assembled.displacement_catalog = std::make_unique<infra::catalogs::FileDisplacementCatalog>(
        infra::catalogs::FileDisplacementCatalogPorts{.logger = logger_factory(logger_context, "IDisplacementCatalog")},
        catalogs_dir + "/displacements"
    );

    assembled.gauge_catalog = std::make_unique<infra::catalogs::FileGaugeCatalog>(
        infra::catalogs::FileGaugeCatalogPorts{.logger = logger_factory(logger_context, "IGaugeCatalog")},
        catalogs_dir + "/gauges"
    );

    assembled.precision_catalog = std::make_unique<infra::catalogs::FilePrecisionCatalog>(
        infra::catalogs::FilePrecisionCatalogPorts{.logger = logger_factory(logger_context, "IPrecisionCatalog")},
        catalogs_dir + "/precision_classes"
    );

    assembled.pressure_unit_catalog = std::make_unique<infra::catalogs::FilePressureUnitCatalog>(
        infra::catalogs::FilePressureUnitCatalogPorts{.logger = logger_factory(logger_context, "IPressureUnitCatalog")},
        catalogs_dir + "/pressure_units"
    );

    assembled.printer_catalog = std::make_unique<infra::catalogs::FilePrinterCatalog>(
        infra::catalogs::FilePrinterCatalogPorts{.logger = logger_factory(logger_context, "IPrinterCatalog")},
        catalogs_dir + "/printers"
    );

    assembled.info_settings_storage = std::make_unique<infra::storage::QtInfoSettingsStorage>(
        "CleanGraduator",
        "CleanGraduator",
        infra::storage::QtInfoSettingsStorageCatalogs {
            .displacement_catalog = *assembled.displacement_catalog,
            .gauge_catalog = *assembled.gauge_catalog,
            .precision_catalog = *assembled.precision_catalog,
            .pressure_unit_catalog = *assembled.pressure_unit_catalog,
            .printer_catalog = *assembled.printer_catalog,
        }
    );

    return assembled;
}
