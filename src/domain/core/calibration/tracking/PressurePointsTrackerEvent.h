#ifndef CLEANGRADUATOR_PRESSUREPOINTSTRACKEREVENT_H
#define CLEANGRADUATOR_PRESSUREPOINTSTRACKEREVENT_H
#include <variant>


namespace domain::common {

    struct PressurePointsTrackerEvent {

        struct PointEntered {
            int index;
        };

        struct PointExited {
            int index;
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