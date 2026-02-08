#ifndef CLEANGRADUATOR_PROCESSRUNNERPORTS_H
#define CLEANGRADUATOR_PROCESSRUNNERPORTS_H


namespace domain::ports {
    class IPressureSource;
    class IAngleSource;
    class IProcessLifecycle;
}

namespace infra::process {
    struct ProcessRunnerPorts {
        domain::ports::IProcessLifecycle& lifecycle;
        domain::ports::IPressureSource& pressure_source;
        domain::ports::IAngleSource& angle_source;
    };
}

#endif //CLEANGRADUATOR_PROCESSRUNNERPORTS_H