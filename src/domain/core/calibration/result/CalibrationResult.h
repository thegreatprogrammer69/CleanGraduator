#ifndef CLEANGRADUATOR_CALIBRATIONRESULT_H
#define CLEANGRADUATOR_CALIBRATIONRESULT_H

#include <vector>
#include <unordered_map>
#include <optional>
#include <stdexcept> // Для исключений
#include "CalibrationCell.h"
#include "CalibrationCellKey.h"
#include "application/models/info/Gauge.h"
#include "domain/core/calibration/common/CalibrationLayout.h"

namespace domain::common {

class CalibrationResult
{
public:
    explicit CalibrationResult(const CalibrationLayout& layout, application::models::Gauge gauge);

    /**
     * @brief Безопасная установка ячейки.
     * Если ключ невалиден, метод просто ничего не сделает (или можно кинуть exception).
     */
    void setCell(const CalibrationCellKey& key, CalibrationCell cell);

    /**
     * @brief Безопасное получение ячейки.
     * Возвращает std::nullopt, если ключ не найден или ячейка не заполнена.
     */
    const std::optional<CalibrationCell>& cell(const CalibrationCellKey& key) const;

    // --- Геттеры остаются прежними ---
    const std::vector<MotorDirection>& directions() const noexcept;
    const std::vector<PointId>& points() const noexcept;
    const std::vector<SourceId>& sources() const noexcept;
    const application::models::Gauge& gauge() const noexcept;
    size_t totalCells() const noexcept;

    bool isReady() const noexcept;
    void markReady() noexcept;

    bool operator==(const CalibrationResult& other) const;

private:
    // Вспомогательный метод для безопасного поиска индекса
    std::optional<size_t> getFlatIndex(const CalibrationCellKey& key) const;

    size_t index(size_t d, size_t p, size_t s) const;

    void buildIndexMaps();

private:
    bool ready_ = false;

    std::vector<MotorDirection> directions_;
    std::vector<PointId> points_;
    std::vector<SourceId> sources_;

    std::unordered_map<MotorDirection, size_t> direction_index_;
    std::unordered_map<PointId, size_t> point_index_;
    std::unordered_map<SourceId, size_t> source_index_;

    std::vector<std::optional<CalibrationCell>> cells_;

    application::models::Gauge gauge_;
};

} // namespace domain::common

#endif