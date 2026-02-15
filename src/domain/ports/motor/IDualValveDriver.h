#ifndef CLEANGRADUATOR_IDUALVALVEDRIVER_H
#define CLEANGRADUATOR_IDUALVALVEDRIVER_H

namespace domain::ports {
    struct IDualValveDriver {
        virtual void moveToIntake() = 0;
        virtual void moveToExhaust() = 0;
        virtual void moveToClosed() = 0;
        virtual ~IDualValveDriver() = default;
    };
}

#endif //CLEANGRADUATOR_IDUALVALVEDRIVER_H