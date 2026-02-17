#include <iostream>
#include <vector>
#include <iomanip>

#include "infrastructure/calibration/motion_controller/stand4/IPressurePointsTrackerObserver.h"
#include "infrastructure/calibration/motion_controller/stand4/PressurePointsTracker.h"

using namespace infra::calib::stand4;
using namespace domain::common;

class TestObserver final : public IPressurePointsTrackerObserver {
public:
    void onPointEntered(int index) override {
        std::cout << "  >>> ENTER point[" << index << "]\n";
    }

    void onPointExited(int index) override {
        std::cout << "  <<< EXIT  point[" << index << "]\n";
    }
};

int main()
{
    TestObserver observer;
    PressurePointsTracker tracker(observer);

    tracker.setEnterThreshold(0.2);  // 20% от шага
    tracker.setExitThreshold(0.35);  // 35% от шага

    std::vector<double> points{0, 10, 20, 30, 40};

    tracker.beginTracking(points, MotorDirection::Forward);

    std::cout << std::fixed << std::setprecision(2);

    for (double pressure = -5.0; pressure <= 50.0; pressure += 0.5) {
        std::cout << "pressure = " << std::setw(6) << pressure << "\n";
        tracker.update(pressure);
    }

    tracker.endTracking();

    return 0;
}
