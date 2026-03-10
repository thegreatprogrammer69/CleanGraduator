#ifndef CLEANGRADUATOR_CALIBRATIONRESULT_H
#define CLEANGRADUATOR_CALIBRATIONRESULT_H

#include <vector>
#include <unordered_map>
#include <optional>
#include <stdexcept> // Для исключений
#include "CalibrationCell.h"
#include "CalibrationCellKey.h"
#include "domain/core/calibration/common/CalibrationLayout.h"

namespace domain::common {

class CalibrationResult
{
public:
    CalibrationResult(const CalibrationLayout& layout)
        : directions_(layout.directions)
        , points_(layout.points)
        , sources_(layout.sources)
    {
        buildIndexMaps();
        cells_.resize(layout.getTotalCells());
    }

    /**
     * @brief Безопасная установка ячейки.
     * Если ключ невалиден, метод просто ничего не сделает (или можно кинуть exception).
     */
    void setCell(const CalibrationCellKey& key, CalibrationCell cell)
    {
        auto idx = getFlatIndex(key);
        if (idx && *idx < cells_.size()) {
            cells_[*idx] = std::move(cell);
        }
    }

    /**
     * @brief Безопасное получение ячейки.
     * Возвращает std::nullopt, если ключ не найден или ячейка не заполнена.
     */
    const std::optional<CalibrationCell>& cell(const CalibrationCellKey& key) const
    {
        auto idx = getFlatIndex(key);
        if (!idx || *idx >= cells_.size()) {
            static const std::optional<CalibrationCell> empty;
            return empty;
        }
        return cells_[*idx];
    }

    // --- Геттеры остаются прежними ---

private:
    // Вспомогательный метод для безопасного поиска индекса
    std::optional<size_t> getFlatIndex(const CalibrationCellKey& key) const
    {
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

    size_t index(size_t d, size_t p, size_t s) const
    {
        return d * points_.size() * sources_.size()
             + p * sources_.size()
             + s;
    }

    void buildIndexMaps()
    {
        for (size_t i = 0; i < directions_.size(); ++i)
            direction_index_[directions_[i]] = i;

        for (size_t i = 0; i < points_.size(); ++i)
            point_index_[points_[i]] = i;

        for (size_t i = 0; i < sources_.size(); ++i)
            source_index_[sources_[i]] = i;
    }

private:
    std::vector<MotorDirection> directions_;
    std::vector<PointId> points_;
    std::vector<SourceId> sources_;

    std::unordered_map<MotorDirection, size_t> direction_index_;
    std::unordered_map<PointId, size_t> point_index_;
    std::unordered_map<SourceId, size_t> source_index_;

    std::vector<std::optional<CalibrationCell>> cells_;
};

} // namespace domain::common

#endif