#ifndef CLEANGRADUATOR_TIMESERIES_H
#define CLEANGRADUATOR_TIMESERIES_H
#include <vector>

namespace domain::common {

    class TimeSeries {
    public:
        TimeSeries() = default;
        ~TimeSeries() = default;

        TimeSeries(
            std::vector<float> time,
            std::vector<float> value
        );

        void clear();
        bool empty() const;
        std::size_t size() const;

        const std::vector<float>& time() const;
        const std::vector<float>& value() const;

        void push(float time, float value);

    private:
        std::vector<float> time_;
        std::vector<float> value_;
    };

    struct AngleSeries : TimeSeries {};
    struct PressureSeries : TimeSeries {};

}


#endif //CLEANGRADUATOR_TIMESERIES_H