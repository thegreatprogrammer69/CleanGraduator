#ifndef CLEANGRADUATOR_TIMESERIES_H
#define CLEANGRADUATOR_TIMESERIES_H
#include <vector>

namespace domain::common {

    class TimeSeries {
    public:
        TimeSeries(
            std::vector<double> time,
            std::vector<double> value
        );

        std::size_t size() const { return time_.size(); }
        const std::vector<double>& time() const { return time_; }
        const std::vector<double>& value() const { return value_; }

        void push(double time, double value) { time_.push_back(time); value_.push_back(value); }

    private:
        std::vector<double> time_;
        std::vector<double> value_;
    };

}


#endif //CLEANGRADUATOR_TIMESERIES_H