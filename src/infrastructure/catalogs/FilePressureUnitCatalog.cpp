#include "FilePressureUnitCatalog.h"

#include <fstream>
#include <locale>
#include <stdexcept>
#include <unordered_map>

namespace infra::catalogs {

using application::models::PressureUnit;

namespace {
const std::unordered_map<std::wstring, domain::common::PressureUnit> unitMap = {
    {L"Pa", domain::common::PressureUnit::Pa},
    {L"kPa", domain::common::PressureUnit::kPa},
    {L"MPa", domain::common::PressureUnit::MPa},
    {L"bar", domain::common::PressureUnit::bar},
    {L"atm", domain::common::PressureUnit::atm},
    {L"mmHg", domain::common::PressureUnit::mmHg},
    {L"mmH2O", domain::common::PressureUnit::mmH2O},
    {L"kgf_cm2", domain::common::PressureUnit::kgf_cm2},
    {L"kgf_m2", domain::common::PressureUnit::kgf_m2}};
}

FilePressureUnitCatalog::FilePressureUnitCatalog(std::string filePath) {
    std::wifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filePath);
    }

    file.imbue(std::locale(""));

    std::wstring line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == L'#') {
            continue;
        }

        auto it = unitMap.find(line);
        if (it == unitMap.end()) {
            continue;
        }

        units_.push_back(PressureUnit{.unit = it->second});
    }
}

std::vector<PressureUnit> FilePressureUnitCatalog::list() const {
    return units_;
}

std::optional<PressureUnit> FilePressureUnitCatalog::at(int idx) const {
    if (idx >= 0 && idx < static_cast<int>(units_.size())) {
        return units_.at(idx);
    }
    return std::nullopt;
}

} // namespace infra::catalogs
