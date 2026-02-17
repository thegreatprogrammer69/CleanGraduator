#include "TimeSeries.h"

#include <stdexcept>
#include <algorithm>

namespace domain::common {

    TimeSeries::TimeSeries(
        std::vector<double> time,
        std::vector<double> value)
        : time_(std::move(time))
        , value_(std::move(value))
    {
        if (time_.size() != value_.size()) {
            throw std::invalid_argument(
                "TimeSeries: time and value vectors must have equal size");
        }

        if (!std::is_sorted(time_.begin(), time_.end())) {
            throw std::invalid_argument(
                "TimeSeries: time vector must be monotonically non-decreasing");
        }
    }

    void TimeSeries::clear() { time_.clear(); value_.clear(); }

    bool TimeSeries::empty() const { return time_.empty(); }

    std::size_t TimeSeries::size() const { return time_.size(); }

    const std::vector<double> & TimeSeries::time() const { return time_; }

    const std::vector<double> & TimeSeries::value() const { return value_; }

    void TimeSeries::push(double time, double value) { time_.push_back(time); value_.push_back(value); }
} // namespace domain::common
