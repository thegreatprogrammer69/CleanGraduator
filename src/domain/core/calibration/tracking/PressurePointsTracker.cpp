#include "PressurePointsTracker.h"

#include <cmath>

using namespace domain::common;

PressurePointsTracker::PressurePointsTracker(IPressurePointsTrackerObserver& observer)
    : observer_(observer)
{
}

PressurePointsTracker::~PressurePointsTracker() = default;

void PressurePointsTracker::setEnterThreshold(float v) {
    enter_threshold_ = v;
}

void PressurePointsTracker::setExitThreshold(float v) {
    exit_threshold_ = v;
}

void PressurePointsTracker::beginTracking(const std::vector<float>& points, MotorDirection direction)
{
    if (points.size() < 2) {
        is_tracking_ = false;
        return;
    }

    direction_ = direction;

    if (direction == MotorDirection::Forward) {
        points_ = points;
    } else {
        points_ = std::vector<float>(points.rbegin(), points.rend());
    }

    base_step_ = std::abs(points_[1] - points_[0]);

    current_index_ = 0;
    inside_ = false;
    is_tracking_ = true;
}



void PressurePointsTracker::endTracking() {
    is_tracking_ = false;
    points_.clear();
    inside_ = false;
    current_index_ = 0;
}

void PressurePointsTracker::update(float pressure)
{
    if (!is_tracking_) return;
    if (current_index_ >= points_.size()) return;

    const float target = points_[current_index_];

    if (!inside_) {
        if (direction_ == MotorDirection::Forward) {
            if (pressure >= target) {
                inside_ = true;
                observer_.onPointEntered(current_index_);
            }
        } else {
            if (pressure <= target) {
                inside_ = true;
                observer_.onPointEntered(current_index_);
            }
        }
    }
    else {
        if (direction_ == MotorDirection::Forward) {
            if (pressure > target) {
                observer_.onPointExited(current_index_);
                inside_ = false;
                ++current_index_;
            }
        } else {
            if (pressure < target) {
                observer_.onPointExited(current_index_);
                inside_ = false;
                ++current_index_;
            }
        }

        if (current_index_ >= points_.size()) {
            is_tracking_ = false;
        }
    }
}

