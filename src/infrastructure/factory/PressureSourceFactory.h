#ifndef CLEANGRADUATOR_PRESSURESOURCEFACTORY_H
#define CLEANGRADUATOR_PRESSURESOURCEFACTORY_H

#include <memory>
#include <string>

#include "domain/ports/inbound/IPressureSource.h"
#include "infrastructure/pressure/dm5002/PressureSourcePorts.h"

namespace infra::repo {

    class PressureSourceFactory final {
    public:
        explicit PressureSourceFactory(
            const std::string& ini_path,
            pressure::PressureSourcePorts ports);

        ~PressureSourceFactory();

        std::unique_ptr<domain::ports::IPressureSource> load();

    private:
        std::string ini_path_;
        pressure::PressureSourcePorts ports_;
    };

}

#endif // CLEANGRADUATOR_PRESSURESOURCEFACTORY_H
