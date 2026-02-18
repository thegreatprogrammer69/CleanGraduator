#ifndef CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H
#define CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H
#include <stack>

#include "IPressurePointsTrackerObserver.h"
#include "domain/core/calibration/calculation/PressurePoints.h"
#include "domain/core/motor/motor/MotorDirection.h"

namespace infra::calib::stand4 {

    class PressurePointsTracker {
    public:
        explicit PressurePointsTracker(IPressurePointsTrackerObserver& observer);
        ~PressurePointsTracker();

        void beginTracking(const std::vector<double>& points, domain::common::MotorDirection direction);

        void endTracking();

        void update(double pressure);

        void setEnterThreshold(double v); // например 0.2
        void setExitThreshold(double v);  // например 0.35 (должен быть > enter)

    private:
        bool is_tracking_ = false;
        domain::common::MotorDirection direction_{};

        std::vector<double> points_;
        IPressurePointsTrackerObserver& observer_;

        std::size_t current_index_ = 0;
        bool inside_ = false;

        double base_step_ = 0.0;
        double enter_threshold_ = 0.2;
        double exit_threshold_  = 0.35;

        double enter_tolerance_ = 0.0;
        double exit_tolerance_  = 0.0;
    };

}


#endif //CLEANGRADUATOR_PRESSUREPOINTSTRACKER_H