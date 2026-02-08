#ifndef CLEANGRADUATOR_SELECTGAUGEUSECASE_H
#define CLEANGRADUATOR_SELECTGAUGEUSECASE_H
#include "application/fmt/fmt.h"

namespace domain::ports {
    struct ILogger;
}

namespace application::ports {
    class IMeasurementContextStore;
    class IGaugeCatalog;
}

namespace application::usecase {

    struct SelectGaugeUseCasePorts {
        domain::ports::ILogger& logger;
        ports::IMeasurementContextStore& store;
        ports::IGaugeCatalog& catalog;
    };

    class SelectGaugeUseCase {
    public:
        explicit SelectGaugeUseCase(SelectGaugeUseCasePorts ports);
        void execute(int idx);

    private:
        fmt::FmtLogger logger_;
        ports::IMeasurementContextStore& store_;
        ports::IGaugeCatalog& catalog_;
    };

}

#endif // CLEANGRADUATOR_SELECTGAUGEUSECASE_H
