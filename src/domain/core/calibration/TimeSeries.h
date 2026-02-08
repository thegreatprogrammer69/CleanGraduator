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

        std::size_t size() const;
        const std::vector<double>& time() const;
        const std::vector<double>& value() const;

    private:
        std::vector<double> time_;
        std::vector<double> value_;
    };

}


#endif //CLEANGRADUATOR_TIMESERIES_H