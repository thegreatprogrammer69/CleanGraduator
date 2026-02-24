#ifndef CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H
#define CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H

namespace domain::common {
    enum class CalibrationLifecycleState {
        Idle,        // ничего не происходит, подписок нет
        Starting,    // подписки есть, ждём мотор started
        Running,     // идёт калибровка
        Stopping,    // ждём мотор stopped (graceful stop)
        Homing,
    };
}

#endif //CLEANGRADUATOR_PROCESSLIFECYCLESTATE_H