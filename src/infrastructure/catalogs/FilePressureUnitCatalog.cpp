#include "FilePressureUnitCatalog.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <stdexcept>
#include <unordered_map>

#include "application/fmt/fmt_application.h"

namespace {
inline std::string to_utf8(const std::wstring& ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
    return conv.to_bytes(ws);
}
}

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

FilePressureUnitCatalog::FilePressureUnitCatalog(FilePressureUnitCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::wifstream file(filePath);
    if (!file.is_open()) {
        logger_.error("Failed to open pressure unit catalog file: {}", filePath);
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
            logger_.error("Unknown pressure unit in line: {}", to_utf8(line));
            continue;
        }

        units_.push_back(PressureUnit{.unit = it->second});
        logger_.info("Loaded pressure unit model: {}", units_.back());
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
