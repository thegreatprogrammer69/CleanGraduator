#include "PressurePointsTracker.h"

#include <cmath>

#include "PressurePointsTrackerEvent.h"
#include "domain/ports/calibration/tracking/IPressurePointsTrackerObserver.h"

using namespace domain::common;
using namespace domain::ports;

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
    if (!is_tracking_ || current_index_ >= points_.size())
        return;

    const float target = points_[current_index_];
    const bool forward = (direction_ == MotorDirection::Forward);

    const auto reached = [&](float p, float t) {
        return forward ? (p >= t) : (p <= t);
    };

    const auto passed = [&](float p, float t) {
        return forward ? (p > t) : (p < t);
    };

    if (!inside_) {
        if (reached(pressure, target)) {
            inside_ = true;

            observer_.onPressurePointsTrackerEvent(
                PressurePointsTrackerEvent{
                    PressurePointsTrackerEvent::PointEntered{
                        static_cast<int>(current_index_)
                    }
                }
            );
        }
        return;
    }

    if (passed(pressure, target)) {

        observer_.onPressurePointsTrackerEvent(
            PressurePointsTrackerEvent{
                PressurePointsTrackerEvent::PointExited{
                    static_cast<int>(current_index_)
                }
            }
        );

        inside_ = false;
        ++current_index_;

        if (current_index_ >= points_.size())
            is_tracking_ = false;
    }
}

