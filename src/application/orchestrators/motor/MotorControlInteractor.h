#ifndef CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H
#define CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H
#include <vector>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "../../../domain/ports/drivers/motor/IMotorDriverObserver.h"


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
        void onMotorStarted() override;
        void onMotorStopped() override;
        void onMotorStartFailed(const domain::common::MotorError &) override;
        void onMotorLimitsStateChanged(domain::common::MotorLimitsState) override;
        void onMotorDirectionChanged(domain::common::MotorDirection) override;

    private:
        void notifyRunning(bool running);

    public:


    private:
        domain::ports::IMotorDriver& driver_;
        std::vector<IMotorControlObserver*> observers_;
    };
}


#endif //CLEANGRADUATOR_MOTORCONTROLINTERACTOR_H