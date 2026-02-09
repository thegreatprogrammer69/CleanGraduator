#ifndef CLEANGRADUATOR_SETTINGSUSECASE_H
#define CLEANGRADUATOR_SETTINGSUSECASE_H
#include <vector>

#include "application/dto/PrinterRecord.h"
#include "application/dto/DisplacementRecord.h"
#include "application/dto/GaugeRecord.h"
#include "application/dto/PressureUnitRecord.h"
#include "application/dto/PrecisionRecord.h"
#include "application/dto/UserSettings.h"
#include "domain/fmt/FmtLogger.h"

namespace domain::ports {
    struct ILogger;
}

namespace application::ports {
    struct ISettingsRepository;
    struct IDisplacementCatalog;
    struct IPrinterCatalog;
    struct IPrecisionCatalog;
    struct IGaugeCatalog;
    struct IPressureUnitCatalog;
}

namespace application::usecases {

    struct SettingsUseCaseCatalogs {
        ports::IPrinterCatalog& printer;
        ports::IDisplacementCatalog& displacement;
        ports::IGaugeCatalog& gauge;
        ports::IPressureUnitCatalog& pressure_unit;
        ports::IPrecisionCatalog& precision;
    };

    class SettingsUseCase {
    public:

        SettingsUseCase(domain::ports::ILogger& logger, ports::ISettingsRepository& repository, SettingsUseCaseCatalogs& catalogs);
        ~SettingsUseCase();

        std::vector<dto::PrinterRecord> printers() const;
        std::vector<dto::DisplacementRecord> displacements() const;
        std::vector<dto::GaugeRecord> gauges() const;
        std::vector<dto::PressureUnitRecord> pressureUnits() const;
        std::vector<dto::PrecisionRecord> precisions() const;

        dto::UserSettings load() const;
        void save(const dto::UserSettings&);

    private:
        mutable fmt::FmtLogger logger_;
        ports::ISettingsRepository& repository_;
        SettingsUseCaseCatalogs& catalogs_;
    };
}

#endif //CLEANGRADUATOR_SETTINGSUSECASE_H