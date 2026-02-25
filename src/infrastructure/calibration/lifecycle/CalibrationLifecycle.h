#ifndef CLEANGRADUATOR_CALIBRATIONLIFECYCLE_H
#define CLEANGRADUATOR_CALIBRATIONLIFECYCLE_H

#include <string>
#include <vector>
#include <algorithm>

#include "domain/ports/calibration/lifecycle/ICalibrationLifecycle.h"
#include "domain/ports/calibration/lifecycle/ICalibrationLifecycleObserver.h"
#include "domain/ports/clock/IClock.h"
#include "infrastructure/clock/SessionClock.h"

namespace infra::lifecycle {

    class CalibrationLifecycle final :
        public domain::ports::ICalibrationLifecycle
    {
    public:
        CalibrationLifecycle();
        ~CalibrationLifecycle() override = default;

        // --- Lifecycle ---
        bool start() override;              // Idle -> Starting
        void markRunning() override;        // Starting -> Running

        bool stop() override;               // Running -> Stopping
        void markIdle() override;           // Stopping -> Idle

        void markError(const std::string& err) override; // Active -> Error

        domain::common::CalibrationLifecycleState state() const override;

        // Optional explicit recovery
        void resetToIdle();                 // Error -> Idle

        // --- Observers ---
        void addObserver(domain::ports::ICalibrationLifecycleObserver&) override;
        void removeObserver(domain::ports::ICalibrationLifecycleObserver&) override;

        // --- Clock ---
        domain::ports::IClock& sessionClock();

        std::string lastError() const override;

    private:
        void notify();

    private:
        using State = domain::common::CalibrationLifecycleState;

        State state_ { State::Stopped };
        std::vector<domain::ports::ICalibrationLifecycleObserver*> observers_;

        clock::SessionClock session_clock_;

        std::string last_error_;
    };

} // namespace infra::lifecycle

#endif // CLEANGRADUATOR_CALIBRATIONLIFECYCLE_H