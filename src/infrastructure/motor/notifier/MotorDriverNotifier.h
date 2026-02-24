#ifndef CLEANGRADUATOR_MOTORDRIVERNOTIFIER_H
#define CLEANGRADUATOR_MOTORDRIVERNOTIFIER_H

#include <vector>
#include <mutex>

namespace domain::common {
    struct MotorDriverEvent;
}

namespace domain::ports {
    struct IMotorDriverObserver;
}

namespace infra::motor {
    class MotorDriverNotifier {
    public:
        MotorDriverNotifier() = default;
        ~MotorDriverNotifier() = default;

        void addObserver(domain::ports::IMotorDriverObserver& observer);
        void removeObserver(domain::ports::IMotorDriverObserver& observer);

        void notify(const domain::common::MotorDriverEvent& event);

    private:
        std::mutex mutex_;
        std::vector<domain::ports::IMotorDriverObserver*> observers_;


    };
}

#endif //CLEANGRADUATOR_MOTORDRIVERNOTIFIER_H