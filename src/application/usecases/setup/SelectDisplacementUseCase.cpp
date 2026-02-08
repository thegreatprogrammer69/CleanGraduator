#include "SelectDisplacementUseCase.h"

#include "application/ports/outbound/catalog/IDisplacementCatalog.h"
#include "application/ports/outbound/state/IMeasurementContextStore.h"

application::usecase::SelectDisplacementUseCase::SelectDisplacementUseCase(
    SelectDisplacementUseCasePorts ports)
    : logger_(ports.logger)
    , store_(ports.store)
    , catalog_(ports.catalog)
{
}

void application::usecase::SelectDisplacementUseCase::execute(int idx) {
    logger_.info("selecting displacement with index {}", idx);

    auto ctx = store_.load();
    ctx.displacement = catalog_.at(idx);
    store_.save(ctx);

    logger_.info("displacement selected successfully: {}", ctx.displacement);
}
