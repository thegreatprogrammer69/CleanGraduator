#ifndef CLEANGRADUATOR_SELECTDISPLACEMENTUSECASE_H
#define CLEANGRADUATOR_SELECTDISPLACEMENTUSECASE_H
#include "application/fmt/fmt.h"


namespace domain::ports {
    struct ILogger;
}

namespace application::ports {
    class IMeasurementContextStore;
    class IDisplacementCatalog;
}

namespace application::usecase {

    struct SelectDisplacementUseCasePorts {
        domain::ports::ILogger& logger;
        ports::IMeasurementContextStore& store;
        ports::IDisplacementCatalog& catalog;
    };

    class SelectDisplacementUseCase {
    public:
        explicit SelectDisplacementUseCase(SelectDisplacementUseCasePorts ports);
        void execute(int idx);

    private:
        fmt::FmtLogger logger_;
        ports::IMeasurementContextStore& store_;
        ports::IDisplacementCatalog& catalog_;
    };

}

#endif // CLEANGRADUATOR_SELECTDISPLACEMENTUSECASE_H
