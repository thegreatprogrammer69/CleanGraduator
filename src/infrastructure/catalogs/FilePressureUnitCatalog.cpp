#include "FilePressureUnitCatalog.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

#include "application/fmt/fmt_application.h"

namespace {
void trim(std::string& s) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}
}

namespace infra::catalogs {

using application::models::PressureUnit;

namespace {
const std::unordered_map<std::string, domain::common::PressureUnit> unitMap = {
    {"Pa", domain::common::PressureUnit::Pa},
    {"kPa", domain::common::PressureUnit::kPa},
    {"MPa", domain::common::PressureUnit::MPa},
    {"bar", domain::common::PressureUnit::bar},
    {"atm", domain::common::PressureUnit::atm},
    {"mmHg", domain::common::PressureUnit::mmHg},
    {"mmH2O", domain::common::PressureUnit::mmH2O},
    {"kgf_cm2", domain::common::PressureUnit::kgf_cm2},
    {"kgf_m2", domain::common::PressureUnit::kgf_m2}};
}

FilePressureUnitCatalog::FilePressureUnitCatalog(FilePressureUnitCatalogPorts ports, std::string filePath)
    : logger_(ports.logger)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        logger_.error("Failed to open pressure unit catalog file: {}", filePath);
        throw std::runtime_error("failed to open file: " + filePath);
    }

    std::string line;
    bool first_line = true;

    while (std::getline(file, line)) {
        if (first_line) {
            first_line = false;
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line.erase(0, 3);
            }
        }

        trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto it = unitMap.find(line);
        if (it == unitMap.end()) {
            logger_.error("Unknown pressure unit in line: {}", line);
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
