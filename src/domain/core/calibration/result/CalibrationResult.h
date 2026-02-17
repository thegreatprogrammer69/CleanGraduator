#ifndef CLEANGRADUATOR_CALIBRATIONRESULT_H
#define CLEANGRADUATOR_CALIBRATIONRESULT_H
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "CalibrationCell.h"
#include "CalibrationCellSeverity.h"

namespace domain::common {

class CalibrationResult {
public:
    CalibrationResult() = default;

    void addMeasurement(double angle) {
        cells_.emplace_back(angle);
    }

    void markLast(CalibrationCellSeverity severity, std::string message) {
        if (cells_.empty()) return;
        cells_.back().mark(severity, std::move(message));
    }

    const std::vector<CalibrationCell>& cells() const {
        return cells_;
    }

    double angleRange() const {
        if (cells_.size() < 2) return 0.0;
        return std::fabs(cells_.front().angle() - cells_.back().angle());
    }

    double nonlinearity() const {
        const std::size_t n = cells_.size();
        if (n < 2) return 0.0;

        // Среднее приращение угла
        const double avrDelta =
            (cells_.back().angle() - cells_.front().angle()) / double(n - 1);

        // Максимальное отклонение Δᵢ от среднего
        double maxD = 0.0;
        for (std::size_t i = 0; i + 1 < n; ++i) {
            const double d =
                cells_[i + 1].angle() - cells_[i].angle();
            const double dev = std::fabs(d - avrDelta);
            if (dev > maxD) {
                maxD = dev;
            }
        }

        // Избежание деления на ноль
        if (std::fabs(avrDelta) < 1e-15) {
            return 0.0;
        }

        return (maxD / std::fabs(avrDelta)) * 100.0;
    }

private:
    std::vector<CalibrationCell> cells_;
};

}

#endif //CLEANGRADUATOR_CALIBRATIONRESULT_H