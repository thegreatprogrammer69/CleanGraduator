#ifndef CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H
#define CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H
#include <vector>

#include "domain/core/motor/motor/MotorDirection.h"
#include "domain/ports/motor/IMotorDriverObserver.h"


namespace domain::ports {
    struct IMotorDriver;
}

namespace application::orchestrators {
    struct IMotorControlObserver;

    class MotorControlInteractor final : domain::ports::IMotorDriverObserver
    {
    public:
        explicit MotorControlInteractor(domain::ports::IMotorDriver& driver);

        ~MotorControlInteractor() override;

        void start(int frequency, domain::common::MotorDirection dir);
        void stop();
        void addObserver(IMotorControlObserver& obs);
        void removeObserver(IMotorControlObserver& obs);

    protected:
        // ===== IMotorDriverObserver =====
        void onStarted() override;
        void onStopped() override;
        void onLimitsStateChanged(domain::common::MotorLimitsState) override;
        void onDirectionChanged(domain::common::MotorDirection) override;
        void onFault(const domain::common::MotorFault&) override;

    private:
        void notifyRunning(bool running);

    private:
        domain::ports::IMotorDriver& driver_;
        std::vector<IMotorControlObserver*> observers_;
    };
}


#endif //CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H