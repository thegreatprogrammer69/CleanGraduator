#include "SettingsUseCase.h"

#include "application/ports/outbound/catalog/IDisplacementCatalog.h"
#include "application/ports/outbound/catalog/IGaugeCatalog.h"
#include "application/ports/outbound/catalog/IPrecisionCatalog.h"
#include "application/ports/outbound/catalog/IPressureUnitCatalog.h"
#include "application/ports/outbound/catalog/IPrinterCatalog.h"
#include "application/ports/outbound/settings/ISettingsRepository.h"
#include "domain/ports/outbound/ILogger.h"

namespace application::usecases {

    SettingsUseCase::SettingsUseCase(
        domain::ports::ILogger& logger,
        ports::ISettingsRepository& repository,
        SettingsUseCaseCatalogs& catalogs
    )
        : logger_(logger)
        , repository_(repository)
        , catalogs_(catalogs)
    {
        logger_.info("SettingsUseCase created");
    }

    SettingsUseCase::~SettingsUseCase() = default;

    std::vector<dto::PrinterRecord> SettingsUseCase::printers() const {
        logger_.info("Loading printer catalog");
        return catalogs_.printer.list();
    }

    std::vector<dto::DisplacementRecord> SettingsUseCase::displacements() const {
        logger_.info("Loading displacement catalog");
        return catalogs_.displacement.list();
    }

    std::vector<dto::GaugeRecord> SettingsUseCase::gauges() const {
        logger_.info("Loading gauge catalog");
        return catalogs_.gauge.list();
    }

    std::vector<dto::PressureUnitRecord> SettingsUseCase::pressureUnits() const {
        logger_.info("Loading pressure unit catalog");
        return catalogs_.pressure_unit.list();
    }

    std::vector<dto::PrecisionRecord> SettingsUseCase::precisions() const {
        logger_.info("Loading precision catalog");
        return catalogs_.precision.list();
    }

    dto::UserSettings SettingsUseCase::load() const {
        logger_.info("Loading user settings");
        return repository_.load();
    }

    void SettingsUseCase::save(const dto::UserSettings& settings) {
        logger_.info("Saving user settings");
        repository_.save(settings);
    }

}
