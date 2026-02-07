#ifndef CLEANGRADUATOR_PROCESSRUNNERPORTS_H
#define CLEANGRADUATOR_PROCESSRUNNERPORTS_H
#include "domain/ports/inbound/IAngleSource.h"
#include "domain/ports/inbound/IPressureSource.h"
#include "domain/ports/inbound/IProcessLifecycle.h"

namespace infra::process {
    struct ProcessRunnerPorts {
        IProcessLifecycle& lifecycle;
        IPressureSource& pressure_source;
        IAngleSource& angle_source;
    };
}

#endif //CLEANGRADUATOR_PROCESSRUNNERPORTS_H