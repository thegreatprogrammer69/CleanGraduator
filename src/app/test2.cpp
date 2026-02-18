// #include <iostream>
// #include <vector>
// #include <iomanip>
//
// #include "infrastructure/calibration/motion_controller/stand4/IPressurePointsTrackerObserver.h"
// #include "infrastructure/calibration/motion_controller/stand4/PressurePointsTracker.h"
//
// using namespace infra::calib::stand4;
// using namespace domain::common;
//
// class TestObserver final : public IPressurePointsTrackerObserver {
// public:
//     void onPointEntered(int index) override {
//         std::cout << "  >>> ENTER point[" << index << "]\n";
//     }
//
//     void onPointExited(int index) override {
//         std::cout << "  <<< EXIT  point[" << index << "]\n";
//     }
// };
//
// int main()
// {
//     TestObserver observer;
//     PressurePointsTracker tracker(observer);
//
//     tracker.setEnterThreshold(0.1);
//     tracker.setExitThreshold(0.075);
//
//     std::vector<double> points{0, 10, 20, 30, 40, 50};
//
//     tracker.beginTracking(points, MotorDirection::Backward);
//
//     std::cout << std::fixed << std::setprecision(2);
//
//     for (double pressure = 60.0; pressure >= 0.0; pressure -= 0.1) {
//         std::cout << "pressure = " << std::setw(6) << pressure << "\n";
//         tracker.update(pressure);
//     }
//
//     tracker.endTracking();
//
//     return 0;
// }
