#ifndef CLEANGRADUATOR_MOTOR_H
#define CLEANGRADUATOR_MOTOR_H

#include <mutex>
#include <stdexcept>

#include "MotorFault.h"
#include "MotorState.h"
#include "domain/ports/motor/IMotorDriverObserver.h"

namespace domain::ports {
    struct IMotorDriver;
}

namespace domain::common {

    class Motor final : ports::IMotorDriverObserver {
    public:
        Motor(ports::IMotorDriver& driver,
              int minFrequency,
              int maxFrequency);

        ~Motor();

        // --- State ---
        MotorState state() const noexcept;
        bool isRunning() const noexcept;
        int currentFrequency() const noexcept;
        MotorFault fault() const noexcept;

        // --- Commands ---
        void startForward(int hz);
        void startBackward(int hz);
        void changeFrequency(int hz);
        void stop();
        void emergencyStop();
        void resetEmergency();

    protected:
        // --- Driver callbacks ---
        void onMotorStopped() override;
        void onMotorEmergencyStop() override;
        void onMotorFault(MotorFault fault) override;

    private:
        void ensureNotEmergency() const;
        void ensureNotRunning() const;
        void ensureRunning() const;
        void ensureValidFrequency(int hz) const;
        void checkForwardLimit() const;
        void checkBackwardLimit() const;

    private:
        ports::IMotorDriver& driver_;

        mutable std::mutex mutex_;

        MotorState state_{MotorState::Idle};
        MotorFault fault_{MotorFault::None};
        int currentFrequency_{0};

        int minFrequency_;
        int maxFrequency_;
    };

}

#endif
