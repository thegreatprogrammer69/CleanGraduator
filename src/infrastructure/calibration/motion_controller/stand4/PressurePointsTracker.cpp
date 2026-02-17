#include "PressurePointsTracker.h"

using namespace infra::calib::stand4;
using namespace domain::common;

PressurePointsTracker::PressurePointsTracker(IPressurePointsTrackerObserver& observer)
    : observer_(observer)
{
}

PressurePointsTracker::~PressurePointsTracker() = default;

void PressurePointsTracker::setEnterThreshold(double v) {
    enter_threshold_ = v;
}

void PressurePointsTracker::setExitThreshold(double v) {
    exit_threshold_ = v;
}

void PressurePointsTracker::beginTracking(const std::vector<double>& points,
                                          MotorDirection direction)
{
    if (points.size() < 2) {
        is_tracking_ = false;
        return;
    }

    if (direction == MotorDirection::Forward) {
        points_ = points;
    } else {
        points_ = std::vector<double>(points.rbegin(), points.rend());
    }

    base_step_ = std::abs(points_[1] - points_[0]);

    if (base_step_ <= 0.0) {
        is_tracking_ = false;
        return;
    }

    // Защита от некорректной конфигурации
    if (exit_threshold_ <= enter_threshold_) {
        exit_threshold_ = enter_threshold_ * 1.5;
    }

    enter_tolerance_ = base_step_ * enter_threshold_;
    exit_tolerance_  = base_step_ * exit_threshold_;

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

void PressurePointsTracker::update(double pressure)
{
    if (!is_tracking_) return;
    if (current_index_ >= points_.size()) return;

    const double target = points_[current_index_];
    const double diff   = std::abs(pressure - target);

    if (!inside_) {
        if (diff <= enter_tolerance_) {
            inside_ = true;
            observer_.onPointEntered(current_index_);
        }
    }
    else {
        if (diff > exit_tolerance_) {
            observer_.onPointExited(current_index_);
            inside_ = false;

            ++current_index_;

            if (current_index_ >= points_.size()) {
                is_tracking_ = false;
            }
        }
    }
}


