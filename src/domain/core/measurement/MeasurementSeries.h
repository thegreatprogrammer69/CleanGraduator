#ifndef CLEANGRADUATOR_MEASUREMENTSERIES_H
#define CLEANGRADUATOR_MEASUREMENTSERIES_H
#include <vector>

#include "Timestamp.h"

namespace domain::common {
    class MeasurementSeries {
    public:
        struct AngleSample {
            AngleSample(double timestamp, double angle) : timestamp(timestamp), angle(angle) {}
            double timestamp;
            double angle;
        };
        struct PressureSample {
            PressureSample(double timestamp, double pressure) : timestamp(timestamp), pressure(pressure) {}
            double timestamp;
            double pressure;
        };

    public:
        void addAngle(double timestamp, double angle) {
            angles_.emplace_back(timestamp, angle);
        }

        void addPressure(double timestamp, double pressure) {
            pressures_.emplace_back(timestamp, pressure);
        }

        const std::vector<AngleSample>& angleSamples() const {
            return angles_;
        }

        const std::vector<PressureSample>& pressureSamples() const {
            return pressures_;
        }

        bool empty() const {
            return angles_.empty() || pressures_.empty();
        }

    private:
        std::vector<AngleSample> angles_;
        std::vector<PressureSample> pressures_;
    };
}

#endif //CLEANGRADUATOR_MEASUREMENTSERIES_H