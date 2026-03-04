#ifndef CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H
#define CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H

#include <cstddef>
#include <vector>

#include "domain/core/calibration/recording/PointId.h"
#include "domain/core/drivers/motor/MotorDirection.h"

namespace infra::calib::tracking {

    struct IPressurePointsTrackerObserver;

    class PressurePointsTracker final {
    public:
        explicit PressurePointsTracker(IPressurePointsTrackerObserver& observer);
        ~PressurePointsTracker();

        void beginTracking(const std::vector<float>& points, domain::common::MotorDirection direction);
        void endTracking();

        void feed(float pressure);

        void setEnterThreshold(float v);
        void setExitThreshold(float v);

    private:
        [[nodiscard]] float computeBaseStep(const std::vector<float>& points) const;

        [[nodiscard]] float enterBoundary(float target) const;
        [[nodiscard]] float exitBoundary(float target) const;

        [[nodiscard]] bool reachedEnter(float pressure, float target) const;
        [[nodiscard]] bool reachedExit(float pressure, float target) const;

        void emitEnter(std::size_t orderedIndex);
        void emitExit(std::size_t orderedIndex);

    private:
        bool is_tracking_ = false;
        bool inside_ = false;

        domain::common::MotorDirection direction_{};

        // Точки в порядке фактического обхода:
        // Forward  -> как пришли
        // Backward -> в обратном порядке
        std::vector<float> ordered_points_;

        // Оригинальные PointId для каждой ordered_points_[i]
        std::vector<domain::common::PointId> ordered_point_ids_;

        // Индекс в ordered_points_
        std::size_t current_index_ = 0;

        // Базовый шаг между соседними точками
        float base_step_ = 0.0f;

        // Доли от шага
        float enter_threshold_ = 0.15f;
        float exit_threshold_  = 0.10f;

        IPressurePointsTrackerObserver& observer_;
    };

}

#endif // CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H