#include "QtInfoSettingsStorage.h"

namespace {
    constexpr const char* GROUP = "InfoSettings";

    constexpr const char* KEY_DISPLACEMENT = "displacement_idx";
    constexpr const char* KEY_GAUGE = "gauge_idx";
    constexpr const char* KEY_PRECISION = "precision_idx";
    constexpr const char* KEY_PRESSURE_UNIT = "pressure_unit_idx";
    constexpr const char* KEY_PRINTER = "printer_idx";
}

using namespace infra::storage;
using namespace application::ports;

QtInfoSettingsStorage::QtInfoSettingsStorage(
    QString organization,
    QString application,
    QtInfoSettingsStorageCatalogs catalogs
)
    : settings_(organization, application)
    , catalogs_(catalogs)
{
}

QtInfoSettingsStorage::~QtInfoSettingsStorage() = default;

InfoSettingsData QtInfoSettingsStorage::loadInfoSettings() {
    settings_.beginGroup(GROUP);

    InfoSettingsData data;
    data.displacement_idx = settings_.value(KEY_DISPLACEMENT, 0).toInt();
    data.gauge_idx = settings_.value(KEY_GAUGE, 0).toInt();
    data.precision_idx = settings_.value(KEY_PRECISION, 0).toInt();
    data.pressure_unit_idx = settings_.value(KEY_PRESSURE_UNIT, 0).toInt();
    data.printer_idx = settings_.value(KEY_PRINTER, 0).toInt();

    settings_.endGroup();

    data.displacement_idx = clampToCatalog(data.displacement_idx, static_cast<int>(catalogs_.displacement_catalog.list().size()));
    data.gauge_idx = clampToCatalog(data.gauge_idx, static_cast<int>(catalogs_.gauge_catalog.list().size()));
    data.precision_idx = clampToCatalog(data.precision_idx, static_cast<int>(catalogs_.precision_catalog.list().size()));
    data.pressure_unit_idx = clampToCatalog(data.pressure_unit_idx, static_cast<int>(catalogs_.pressure_unit_catalog.list().size()));
    data.printer_idx = clampToCatalog(data.printer_idx, static_cast<int>(catalogs_.printer_catalog.list().size()));

    return data;
}

void QtInfoSettingsStorage::saveInfoSettings(const InfoSettingsData& data) {
    settings_.beginGroup(GROUP);

    settings_.setValue(KEY_DISPLACEMENT, data.displacement_idx);
    settings_.setValue(KEY_GAUGE, data.gauge_idx);
    settings_.setValue(KEY_PRECISION, data.precision_idx);
    settings_.setValue(KEY_PRESSURE_UNIT, data.pressure_unit_idx);
    settings_.setValue(KEY_PRINTER, data.printer_idx);

    settings_.endGroup();
    settings_.sync();
}

int QtInfoSettingsStorage::clampToCatalog(int idx, int catalog_size) const {
    if (catalog_size <= 0) {
        return -1;
    }

    if (idx < 0 || idx >= catalog_size) {
        return 0;
    }

    return idx;
}
