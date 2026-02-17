#ifndef CLEANGRADUATOR_CALIBRATIONDATASET_H
#define CLEANGRADUATOR_CALIBRATIONDATASET_H
#include <map>

#include "TimeSeries.h"
#include "domain/core/angle/AngleSourceId.h"

namespace domain::common {
    class CalibrationDataset final {
    public:
        CalibrationDataset();

        CalibrationDataset(std::map<AngleSourceId, AngleSeries> angleSeries,
                           PressureSeries pressures);

        void clear();
        void pushAgle(AngleSourceId id, double time, double value);
        void pushPressure(double time, double value);
        bool empty() const noexcept;
        std::vector<AngleSourceId> angleSourceIds() const noexcept;
        const PressureSeries& pressureSeries() const noexcept;
        const AngleSeries& angleSeries(AngleSourceId id) const;

    private:
        std::map<AngleSourceId, AngleSeries> angles_;
        PressureSeries pressures_;
    };
}

#endif //CLEANGRADUATOR_CALIBRATIONDATASET_H