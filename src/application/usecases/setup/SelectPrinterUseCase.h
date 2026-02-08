#ifndef CLEANGRADUATOR_SELECTPRINTERUSECASE_H
#define CLEANGRADUATOR_SELECTPRINTERUSECASE_H
#include "application/fmt/fmt.h"


namespace domain::ports {
    struct ILogger;
}

namespace application::ports {
    class IMeasurementContextStore;
    class IPrinterCatalog;
}

namespace application::usecase {
    struct SelectPrinterUseCasePorts {
        domain::ports::ILogger& logger;
        ports::IMeasurementContextStore& store;
        ports::IPrinterCatalog& catalog;
    };

    class SelectPrinterUseCase {
    public:
        explicit SelectPrinterUseCase(SelectPrinterUseCasePorts ports);
        void execute(int idx);
    private:
        fmt::FmtLogger logger_;
        ports::IMeasurementContextStore& store_;
        ports::IPrinterCatalog& catalog_;
    };
}


#endif //CLEANGRADUATOR_SELECTPRINTERUSECASE_H