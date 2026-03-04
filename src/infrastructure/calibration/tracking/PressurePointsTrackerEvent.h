#ifndef CLEANGRADUATOR_PRESSUREPOINTSTRACKEREVENT_H
#define CLEANGRADUATOR_PRESSUREPOINTSTRACKEREVENT_H
#include <variant>

#include "domain/core/calibration/recording/PointId.h"

namespace infra::calib::tracking {

    struct PressurePointsTrackerEvent {

        struct PointEntered {
            domain::common::PointId index;
        };

        struct PointExited {
            domain::common::PointId index;
        };

        using Data = std::variant<
            PointEntered,
            PointExited
        >;

        Data data;

        explicit PressurePointsTrackerEvent(Data d)
            : data(std::move(d)) {}
    };

}
#endif //CLEANGRADUATOR_PRESSUREPOINTSTRACKEREVENT_H