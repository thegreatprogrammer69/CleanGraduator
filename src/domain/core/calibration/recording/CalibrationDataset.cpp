#include "CalibrationDataset.h"

using namespace domain::common;

CalibrationDataset::CalibrationDataset() = default;

CalibrationDataset::CalibrationDataset(std::map<AngleSourceId, AngleSeries> angleSeries, PressureSeries pressures): angles_(std::move(angleSeries))
    , pressures_(std::move(pressures)) {}

void CalibrationDataset::clear() {
    angles_.clear();
    pressures_.clear();
}

void CalibrationDataset::pushAgle(AngleSourceId id, double time, double value) {
    angles_.at(id).push(time, value);
}

void CalibrationDataset::pushPressure(double time, double value) {
    pressures_.push(time, value);
}

bool CalibrationDataset::empty() const noexcept {
    if (!pressures_.empty())
        return false;

    for (const auto& [_, v] : angles_)
        if (!v.empty())
            return false;

    return true;
}

std::vector<AngleSourceId> CalibrationDataset::angleSourceIds() const noexcept {
    std::vector<AngleSourceId> result;
    result.reserve(angles_.size());
    for (const auto& angle_series : angles_) {
        result.push_back(angle_series.first);
    }
    return result;
}

const PressureSeries & CalibrationDataset::pressureSeries() const noexcept {
    return pressures_;
}

const AngleSeries & CalibrationDataset::angleSeries(AngleSourceId id) const {
    return angles_.at(id);
}
