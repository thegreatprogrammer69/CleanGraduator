#ifndef CLEANGRADUATOR_IDUALVALVEDRIVER_H
#define CLEANGRADUATOR_IDUALVALVEDRIVER_H

namespace domain::ports {
    struct IDualValveDriver {
        virtual void openInputFlap() = 0;
        virtual void openOutputFlap() = 0;
        virtual void closeFlaps() = 0;
        virtual ~IDualValveDriver() = default;
    };
}

#endif //CLEANGRADUATOR_IDUALVALVEDRIVER_H