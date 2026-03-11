#include "CalibrationResult.h"

domain::common::CalibrationResult::CalibrationResult(const CalibrationLayout &layout): directions_(layout.directions)
    , points_(layout.points)
    , sources_(layout.sources) {
    buildIndexMaps();
    cells_.resize(layout.getTotalCells());
}

void domain::common::CalibrationResult::setCell(const CalibrationCellKey &key, CalibrationCell cell) {
    auto idx = getFlatIndex(key);
    if (!idx)
        return;

    if (!cells_[*idx].has_value())
        ++filled_cells_;

    cells_[*idx] = std::move(cell);
}

const std::optional<domain::common::CalibrationCell> & domain::common::CalibrationResult::cell(
    const CalibrationCellKey &key) const {
    auto idx = getFlatIndex(key);
    if (!idx || *idx >= cells_.size()) {
        static const std::optional<CalibrationCell> empty;
        return empty;
    }
    return cells_[*idx];
}

const std::vector<domain::common::MotorDirection> & domain::common::CalibrationResult::directions() const noexcept {
    return directions_;
}

const std::vector<domain::common::PointId> & domain::common::CalibrationResult::points() const noexcept {
    return points_;
}

const std::vector<domain::common::SourceId> & domain::common::CalibrationResult::sources() const noexcept {
    return sources_;
}

size_t domain::common::CalibrationResult::totalCells() const noexcept {
    return cells_.size();
}

bool domain::common::CalibrationResult::isReady() const noexcept { return ready_; }

void domain::common::CalibrationResult::markReady() noexcept { ready_ = true; }

bool domain::common::CalibrationResult::operator==(const CalibrationResult &other) const {
    return cells_ == other.cells_;
}

std::optional<size_t> domain::common::CalibrationResult::getFlatIndex(const CalibrationCellKey &key) const {
    try {
        // Используем find вместо at, чтобы избежать исключений внутри логики
        auto d_it = direction_index_.find(key.direction);
        auto p_it = point_index_.find(key.point_id);
        auto s_it = source_index_.find(key.source_id);

        if (d_it == direction_index_.end() ||
            p_it == point_index_.end() ||
            s_it == source_index_.end())
        {
            return std::nullopt;
        }

        return index(d_it->second, p_it->second, s_it->second);
    } catch (...) {
        return std::nullopt;
    }
}

size_t domain::common::CalibrationResult::index(size_t d, size_t p, size_t s) const {
    return d * points_.size() * sources_.size()
           + p * sources_.size()
           + s;
}

void domain::common::CalibrationResult::buildIndexMaps() {
    for (size_t i = 0; i < directions_.size(); ++i)
        direction_index_[directions_[i]] = i;

    for (size_t i = 0; i < points_.size(); ++i)
        point_index_[points_[i]] = i;

    for (size_t i = 0; i < sources_.size(); ++i)
        source_index_[sources_[i]] = i;
}
