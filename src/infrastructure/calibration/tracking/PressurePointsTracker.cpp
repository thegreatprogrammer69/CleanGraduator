#include "PressurePointsTracker.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "IPressurePointsTrackerObserver.h"
#include "PressurePointsTrackerEvent.h"

using namespace domain::common;
using namespace infra::calib::tracking;

PressurePointsTracker::PressurePointsTracker(IPressurePointsTrackerObserver& observer)
    : observer_(observer)
{
}

PressurePointsTracker::~PressurePointsTracker() = default;

void PressurePointsTracker::setEnterThreshold(float v) {
    enter_threshold_ = std::max(0.0f, v);
}

void PressurePointsTracker::setExitThreshold(float v) {
    exit_threshold_ = std::max(0.0f, v);
}

float PressurePointsTracker::computeBaseStep(const std::vector<float>& points) const
{
    if (points.size() < 2) {
        return 0.0f;
    }

    float min_step = std::numeric_limits<float>::max();

    for (std::size_t i = 1; i < points.size(); ++i) {
        const float step = std::fabs(points[i] - points[i - 1]);
        if (step > 0.0f) {
            min_step = std::min(min_step, step);
        }
    }

    if (min_step == std::numeric_limits<float>::max()) {
        return 0.0f;
    }

    return min_step;
}

void PressurePointsTracker::beginTracking(const std::vector<float>& points, MotorDirection direction)
{
    endTracking();

    if (points.size() < 2) {
        return;
    }

    const float step = computeBaseStep(points);
    if (step <= 0.0f) {
        return;
    }

    direction_ = direction;
    base_step_ = step;

    ordered_points_.reserve(points.size());
    ordered_point_ids_.reserve(points.size());

    if (direction == MotorDirection::Forward) {
        for (std::size_t i = 0; i < points.size(); ++i) {
            ordered_points_.push_back(points[i]);
            ordered_point_ids_.push_back(PointId(i, points[i]));
        }
    } else {
        for (std::size_t i = points.size(); i-- > 0;) {
            ordered_points_.push_back(points[i]);
            ordered_point_ids_.push_back(PointId(i, points[i]));
        }
    }

    current_index_ = 0;
    inside_ = false;
    is_tracking_ = true;
}

void PressurePointsTracker::endTracking()
{
    is_tracking_ = false;
    inside_ = false;
    current_index_ = 0;
    base_step_ = 0.0f;

    ordered_points_.clear();
    ordered_point_ids_.clear();
}

float PressurePointsTracker::enterBoundary(float target) const
{
    const float delta = base_step_ * enter_threshold_;

    if (direction_ == MotorDirection::Forward) {
        return target - delta;
    }

    return target + delta;
}

float PressurePointsTracker::exitBoundary(float target) const
{
    const float delta = base_step_ * exit_threshold_;

    if (direction_ == MotorDirection::Forward) {
        return target + delta;
    }

    return target - delta;
}

bool PressurePointsTracker::reachedEnter(float pressure, float target) const
{
    const float boundary = enterBoundary(target);

    if (direction_ == MotorDirection::Forward) {
        return pressure >= boundary;
    }

    return pressure <= boundary;
}

bool PressurePointsTracker::reachedExit(float pressure, float target) const
{
    const float boundary = exitBoundary(target);

    if (direction_ == MotorDirection::Forward) {
        return pressure >= boundary;
    }

    return pressure <= boundary;
}

void PressurePointsTracker::emitEnter(std::size_t orderedIndex)
{
    observer_.onPressurePointsTrackerEvent(
        PressurePointsTrackerEvent{
            PressurePointsTrackerEvent::PointEntered{
                ordered_point_ids_[orderedIndex]
            }
        }
    );
}

void PressurePointsTracker::emitExit(std::size_t orderedIndex)
{
    observer_.onPressurePointsTrackerEvent(
        PressurePointsTrackerEvent{
            PressurePointsTrackerEvent::PointExited{
                ordered_point_ids_[orderedIndex]
            }
        }
    );
}

void PressurePointsTracker::feed(float pressure)
{
    if (!is_tracking_) {
        return;
    }

    while (is_tracking_ && current_index_ < ordered_points_.size()) {
        const float target = ordered_points_[current_index_];

        // Ещё не вошли в окно текущей точки
        if (!inside_) {
            if (!reachedEnter(pressure, target)) {
                return;
            }

            inside_ = true;
            emitEnter(current_index_);
        }

        // Уже внутри окна: проверяем выход
        if (!reachedExit(pressure, target)) {
            return;
        }

        emitExit(current_index_);
        inside_ = false;
        ++current_index_;

        if (current_index_ >= ordered_points_.size()) {
            is_tracking_ = false;
            return;
        }

        // Не return: тот же pressure может сразу перескочить и следующую точку тоже
    }
}