#ifndef CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H
#define CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H
#include <vector>

#include "domain/core/drivers/motor/MotorDirection.h"

namespace domain::ports {
    struct IPressurePointsTrackerObserver;
}

namespace domain::common {

    class PressurePointsTracker {
    public:
        explicit PressurePointsTracker(ports::IPressurePointsTrackerObserver& observer);
        ~PressurePointsTracker();

        void beginTracking(const std::vector<float>& points, MotorDirection direction);

        void endTracking();

        void update(float pressure);

        void setEnterThreshold(float v);
        void setExitThreshold(float v);

    private:
        bool is_tracking_ = false;
        MotorDirection direction_{};

        std::vector<float> points_;
        ports::IPressurePointsTrackerObserver& observer_;

        std::size_t current_index_ = 0;
        bool inside_ = false;

        float base_step_ = 0.0;
        float enter_threshold_ = 0.15;
        float exit_threshold_  = 0.1;
    };

}


#endif //CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H