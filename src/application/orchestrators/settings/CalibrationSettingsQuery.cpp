#include "CalibrationSettingsQuery.h"
#include "application/ports/settings/IInfoSettingsStorage.h"
#include "application/ports/catalogs/IDisplacementCatalog.h"
#include "application/ports/catalogs/IGaugeCatalog.h"
#include "application/ports/catalogs/IGaugePrecisionCatalog.h"
#include "application/ports/catalogs/IPressureUnitCatalog.h"
#include "application/ports/catalogs/IPrinterCatalog.h"

using namespace application::orchestrators;
using namespace application::models;

CalibrationSettingsQuery::CalibrationSettingsQuery(CalibrationSettingsQueryPorts ports)
    : storage_(ports.storage)
    , displacement_catalog_(ports.displacement_catalog)
    , gauge_catalog_(ports.gauge_catalog)
    , precision_catalog_(ports.precision_catalog)
    , pressure_unit_catalog_(ports.pressure_unit_catalog)
    , printer_catalog_(ports.printer_catalog) {
    load();
}

std::optional<domain::common::PressurePoints> CalibrationSettingsQuery::currentGaugePressurePoints() const {
    if (auto gauge = gauge_catalog_.at(data_.gauge_idx)) {
        return gauge->points.poi fdsfsd  sdfnts;
    }
    return std::nullopt;
}

std::optional<domain::common::GaugePrecision> CalibrationSettingsQuery::currentPrecision() const {
    if (auto prec = precision_catalog_.at(data_.precision_idx)) {
        return prec->precision;
    }
    return std::nullopt;
}

std::optional<domain::common::PressureUnit> CalibrationSettingsQuery::currentPressureUnit() const {
    if (auto pu = pressure_unit_catalog_.at(data_.pressure_unit_idx)) {
        return pu->unit;
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> CalibrationSettingsQuery::currentSaveResultPath() const {
    // TODO
    return std::nullopt;
}


void CalibrationSettingsQuery::load() {
    data_ = storage_.loadInfoSettings();
    normalize();
}


void CalibrationSettingsQuery::normalize() {
    if (!displacement_catalog_.at(data_.displacement_idx))
        data_.displacement_idx = 0;

    if (!gauge_catalog_.at(data_.gauge_idx))
        data_.gauge_idx = 0;

    if (!precision_catalog_.at(data_.precision_idx))
        data_.precision_idx = 0;

    if (!pressure_unit_catalog_.at(data_.pressure_unit_idx))
        data_.pressure_unit_idx = 0;

    if (!printer_catalog_.at(data_.printer_idx))
        data_.printer_idx = 0;
}
