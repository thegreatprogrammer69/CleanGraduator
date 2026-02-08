#include "SelectPrinterUseCase.h"
#include "application/ports/outbound/catalog/IPrinterCatalog.h"
#include "application/ports/outbound/state/IMeasurementContextStore.h"
#include "domain/ports/outbound/ILogger.h"

application::usecase::SelectPrinterUseCase::SelectPrinterUseCase(SelectPrinterUseCasePorts ports)
    : logger_(ports.logger)
    , store_(ports.store)
    , catalog_(ports.catalog)
{ }

void application::usecase::SelectPrinterUseCase::execute(int idx) {
    logger_.info("selecting printer with index {}", idx);

    auto ctx = store_.load();
    ctx.printer = catalog_.at(idx);;
    store_.save(ctx);

    logger_.info("printer selected successfully: {}", ctx.printer);
}
