#include "InfoSettingsViewModel.h"

#include "domain/core/measurement/PressureUnit.h"

#include <codecvt>
#include <locale>

using namespace mvvm;
using namespace application::ports;

InfoSettingsViewModel::InfoSettingsViewModel(InfoSettingsViewModelDeps deps)
    : deps_(deps)
{
    loadCatalogs();
    loadSelection();
}

const std::vector<std::string>& InfoSettingsViewModel::displacements() const { return displacements_; }
const std::vector<std::string>& InfoSettingsViewModel::gauges() const { return gauges_; }
const std::vector<std::string>& InfoSettingsViewModel::precisions() const { return precisions_; }
const std::vector<std::string>& InfoSettingsViewModel::pressureUnits() const { return pressure_units_; }
const std::vector<std::string>& InfoSettingsViewModel::printers() const { return printers_; }

void InfoSettingsViewModel::save() {
    const InfoSettingsData data {
        .displacement_idx = selectedDisplacement.get_copy(),
        .gauge_idx = selectedGauge.get_copy(),
        .precision_idx = selectedPrecision.get_copy(),
        .pressure_unit_idx = selectedPressureUnit.get_copy(),
        .printer_idx = selectedPrinter.get_copy(),
    };

    deps_.storage.saveInfoSettings(data);
}

void InfoSettingsViewModel::loadCatalogs() {
    for (const auto& item : deps_.displacement_catalog.list()) {
        displacements_.emplace_back(toUtf8(item.name));
    }

    for (const auto& item : deps_.gauge_catalog.list()) {
        gauges_.emplace_back(toUtf8(item.name));
    }

    for (const auto& item : deps_.precision_catalog.list()) {
        precisions_.emplace_back(std::to_string(item.precision));
    }

    for (const auto& item : deps_.pressure_unit_catalog.list()) {
        switch (item.unit) {
            case domain::common::PressureUnit::Pa: pressure_units_.emplace_back("Pa"); break;
            case domain::common::PressureUnit::kPa: pressure_units_.emplace_back("kPa"); break;
            case domain::common::PressureUnit::MPa: pressure_units_.emplace_back("MPa"); break;
            case domain::common::PressureUnit::bar: pressure_units_.emplace_back("bar"); break;
            case domain::common::PressureUnit::atm: pressure_units_.emplace_back("atm"); break;
            case domain::common::PressureUnit::mmHg: pressure_units_.emplace_back("mmHg"); break;
            case domain::common::PressureUnit::mmH2O: pressure_units_.emplace_back("mmH2O"); break;
            case domain::common::PressureUnit::kgf_cm2: pressure_units_.emplace_back("kgf/cm²"); break;
            case domain::common::PressureUnit::kgf_m2: pressure_units_.emplace_back("kgf/m²"); break;
        }
    }

    for (const auto& item : deps_.printer_catalog.list()) {
        printers_.emplace_back(toUtf8(item.name));
    }
}

void InfoSettingsViewModel::loadSelection() {
    const InfoSettingsData data = deps_.storage.loadInfoSettings();

    selectedDisplacement.set(data.displacement_idx);
    selectedGauge.set(data.gauge_idx);
    selectedPrecision.set(data.precision_idx);
    selectedPressureUnit.set(data.pressure_unit_idx);
    selectedPrinter.set(data.printer_idx);
}

std::string InfoSettingsViewModel::toUtf8(const std::wstring& value) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(value);
}
