#include "CalibrationContextProvider.h"

#include "application/ports/catalogs/IDisplacementCatalog.h"
#include "application/ports/catalogs/IGaugeCatalog.h"
#include "application/ports/catalogs/IGaugePrecisionCatalog.h"
#include "application/ports/catalogs/IPressureUnitCatalog.h"
#include "application/ports/catalogs/IPrinterCatalog.h"

using namespace application::orchestrators;
using namespace application::models;

CalibrationContextProvider::CalibrationContextProvider(CalibrationContextProviderPorts ports)
    : storage_(ports.storage)
    , displacement_catalog_(ports.displacement_catalog)
    , gauge_catalog_(ports.gauge_catalog)
    , precision_catalog_(ports.precision_catalog)
    , pressure_unit_catalog_(ports.pressure_unit_catalog)
    , printer_catalog_(ports.printer_catalog)
{
}

std::optional<CalibrationContext> CalibrationContextProvider::current() const
{
    auto data = storage_.loadInfoSettings();
    normalize(data);

    const auto displacement = displacement_catalog_.at(data.displacement_idx);
    const auto gauge = gauge_catalog_.at(data.gauge_idx);
    const auto precision = precision_catalog_.at(data.precision_idx);
    const auto pressure_unit = pressure_unit_catalog_.at(data.pressure_unit_idx);
    const auto printer = printer_catalog_.at(data.printer_idx);

    if (!displacement || !gauge || !precision || !pressure_unit || !printer)
        return std::nullopt;

    return CalibrationContext{
        *displacement,
        *gauge,
        *precision,
        *pressure_unit,
        *printer
    };
}

std::optional<domain::common::PressurePoints>
CalibrationContextProvider::currentGaugePressurePoints() const
{
    const auto calibration_context = current();
    if (!calibration_context)
        return std::nullopt;

    if (calibration_context->gauge.points.value.size() < 2)
        return std::nullopt;

    domain::common::PressurePoints points;
    for (auto point : calibration_context->gauge.points.value)
    {
        points.value.push_back(domain::common::Pressure(
            point,
            calibration_context->pressure_unit.unit));
    }

    return points;
}

void CalibrationContextProvider::normalize(ports::InfoSettingsData& data) const
{
    if (!displacement_catalog_.at(data.displacement_idx))
        data.displacement_idx = 0;

    if (!gauge_catalog_.at(data.gauge_idx))
        data.gauge_idx = 0;

    if (!precision_catalog_.at(data.precision_idx))
        data.precision_idx = 0;

    if (!pressure_unit_catalog_.at(data.pressure_unit_idx))
        data.pressure_unit_idx = 0;

    if (!printer_catalog_.at(data.printer_idx))
        data.printer_idx = 0;
}
