#include "QtInfoSettingsStorage.h"
#include <QDebug>
#include <cmath>

namespace {
    constexpr const char* GROUP = "InfoSettings";

    constexpr const char* KEY_DISPLACEMENT = "displacement_idx";
    constexpr const char* KEY_GAUGE = "gauge_idx";
    constexpr const char* KEY_PRECISION = "precision_idx";
    constexpr const char* KEY_PRESSURE_UNIT = "pressure_unit_idx";
    constexpr const char* KEY_PRINTER = "printer_idx";
    constexpr const char* KEY_KU_ENABLED = "ku_enabled";
    constexpr const char* KEY_CENTERED_LABEL_ENABLED = "centered_label_enabled";
    constexpr const char* KEY_MAX_CENTER_DEVIATION_DEG = "max_center_deviation_deg";
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
    settings_.sync();
    settings_.beginGroup(GROUP);

    InfoSettingsData data;
    data.displacement_idx = settings_.value(KEY_DISPLACEMENT, 0).toInt();
    data.gauge_idx = settings_.value(KEY_GAUGE, 0).toInt();
    data.precision_idx = settings_.value(KEY_PRECISION, 0).toInt();
    data.pressure_unit_idx = settings_.value(KEY_PRESSURE_UNIT, 0).toInt();
    data.printer_idx = settings_.value(KEY_PRINTER, 0).toInt();
    data.ku_enabled = settings_.value(KEY_KU_ENABLED, false).toBool();
    data.centered_label_enabled = settings_.value(KEY_CENTERED_LABEL_ENABLED, false).toBool();
    data.max_center_deviation_deg = settings_.value(KEY_MAX_CENTER_DEVIATION_DEG, 0.9F).toFloat();

    settings_.endGroup();

    if (!std::isfinite(data.max_center_deviation_deg) || data.max_center_deviation_deg < 0.0F) {
        data.max_center_deviation_deg = 0.9F;
    }

    data.displacement_idx = clampToCatalog(data.displacement_idx, static_cast<int>(catalogs_.displacement_catalog.list().size()));
    data.gauge_idx = clampToCatalog(data.gauge_idx, static_cast<int>(catalogs_.gauge_catalog.list().size()));
    data.precision_idx = clampToCatalog(data.precision_idx, static_cast<int>(catalogs_.precision_catalog.list().size()));
    data.pressure_unit_idx = clampToCatalog(data.pressure_unit_idx, static_cast<int>(catalogs_.pressure_unit_catalog.list().size()));
    data.printer_idx = clampToCatalog(data.printer_idx, static_cast<int>(catalogs_.printer_catalog.list().size()));

    return data;
}

void QtInfoSettingsStorage::saveInfoSettings(const InfoSettingsData& data) {
    settings_.beginGroup(GROUP);
    // qDebug() << "Settings file:" << settings_.fileName();
    // qDebug() << "Status:" << settings_.status();
    settings_.setValue(KEY_DISPLACEMENT, data.displacement_idx);
    settings_.setValue(KEY_GAUGE, data.gauge_idx);
    settings_.setValue(KEY_PRECISION, data.precision_idx);
    settings_.setValue(KEY_PRESSURE_UNIT, data.pressure_unit_idx);
    settings_.setValue(KEY_PRINTER, data.printer_idx);
    settings_.setValue(KEY_KU_ENABLED, data.ku_enabled);
    settings_.setValue(KEY_CENTERED_LABEL_ENABLED, data.centered_label_enabled);
    settings_.setValue(KEY_MAX_CENTER_DEVIATION_DEG, data.max_center_deviation_deg);

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
