#ifndef CLEANGRADUATOR_IMOTORCONTROLOBSERVER_H
#define CLEANGRADUATOR_IMOTORCONTROLOBSERVER_H

namespace application::orchestrators {
    struct IMotorControlObserver {
        virtual ~IMotorControlObserver() = default;
        virtual void onRunningChanged(bool) = 0;
    };
}

#endif //CLEANGRADUATOR_IMOTORCONTROLOBSERVER_H