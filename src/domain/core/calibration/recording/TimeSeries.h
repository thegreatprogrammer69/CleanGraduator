#ifndef CLEANGRADUATOR_TIMESERIES_H
#define CLEANGRADUATOR_TIMESERIES_H
#include <vector>

namespace domain::common {

    class TimeSeries {
    public:
        TimeSeries() = default;

        TimeSeries(
            std::vector<double> time,
            std::vector<double> value
        );

        void clear();
        bool empty() const;
        std::size_t size() const;

        const std::vector<double>& time() const;
        const std::vector<double>& value() const;

        void push(double time, double value);

    private:
        std::vector<double> time_;
        std::vector<double> value_;
    };

    struct AngleSeries : TimeSeries {};
    struct PressureSeries : TimeSeries {};

}


#endif //CLEANGRADUATOR_TIMESERIES_H