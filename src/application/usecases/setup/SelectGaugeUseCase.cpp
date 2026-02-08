#include "SelectGaugeUseCase.h"

#include "application/ports/outbound/catalog/IGaugeCatalog.h"
#include "application/ports/outbound/state/IMeasurementContextStore.h"

application::usecase::SelectGaugeUseCase::SelectGaugeUseCase(
    SelectGaugeUseCasePorts ports)
    : logger_(ports.logger)
    , store_(ports.store)
    , catalog_(ports.catalog)
{
}

void application::usecase::SelectGaugeUseCase::execute(int idx) {
    logger_.info("selecting gauge with index {}", idx);

    auto ctx = store_.load();
    ctx.gauge = catalog_.at(idx);
    store_.save(ctx);

    logger_.info("gauge selected successfully: {}", ctx.gauge);
}
