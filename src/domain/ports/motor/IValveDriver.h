#ifndef CLEANGRADUATOR_IDUALVALVEDRIVER_H
#define CLEANGRADUATOR_IDUALVALVEDRIVER_H

namespace domain::ports {
    struct IValveDriverObserver;

    struct IValveDriver {
        virtual void openInputFlap() = 0;
        virtual void openOutputFlap() = 0;
        virtual void closeFlaps() = 0;

        virtual void addObserver(IValveDriverObserver&) = 0;
        virtual void removeObserver(IValveDriverObserver&) = 0;

        virtual ~IValveDriver() = default;
    };
}

#endif //CLEANGRADUATOR_IDUALVALVEDRIVER_H