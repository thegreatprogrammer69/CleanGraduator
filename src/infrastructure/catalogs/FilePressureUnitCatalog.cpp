#include "FilePressureUnitCatalog.h"

#include <fstream>
#include <locale>
#include <unordered_map>
#include <stdexcept>

namespace infra::catalogs {

    using application::model::PressureUnitRecord;
    using domain::common::PressureUnit;

    namespace {
        const std::unordered_map<std::wstring, PressureUnit> unitMap = {
            {L"Pa",       PressureUnit::Pa},
            {L"kPa",      PressureUnit::kPa},
            {L"MPa",      PressureUnit::MPa},
            {L"bar",      PressureUnit::bar},
            {L"atm",      PressureUnit::atm},
            {L"mmHg",     PressureUnit::mmHg},
            {L"mmH2O",    PressureUnit::mmH2O},
            {L"kgf_cm2",  PressureUnit::kgf_cm2},
            {L"kgf_m2",   PressureUnit::kgf_m2}
        };
    }

    FilePressureUnitCatalog::FilePressureUnitCatalog(std::string filePath) {
        std::wifstream file(filePath);
        if (!file.is_open())
            throw std::runtime_error("failed to open file: " + filePath);

        file.imbue(std::locale(""));

        std::wstring line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (!line.empty() && line[0] == L'#') continue; // комментарий

            auto it = unitMap.find(line);
            if (it == unitMap.end())
                continue; // неизвестная единица — пропускаем

            PressureUnitRecord record;
            record.unit = it->second;

            units_.push_back(std::move(record));
        }
    }

    std::vector<PressureUnitRecord> FilePressureUnitCatalog::list() const {
        return units_;
    }

    std::optional<PressureUnitRecord> FilePressureUnitCatalog::at(int idx) const {
        if (idx >= 0 && idx < static_cast<int>(units_.size())) {
            return units_.at(idx);
        }
        return std::nullopt;
    }

}
