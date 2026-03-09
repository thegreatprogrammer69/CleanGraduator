#ifndef CLEANGRADUATOR_ANGLESOURCEFACTORY_H
#define CLEANGRADUATOR_ANGLESOURCEFACTORY_H

#include <memory>
#include <string>

#include "domain/ports/angle/IAngleSource.h"
#include "../angle/AngleSourcePorts.h"
#include "domain/core/angle/SourceId.h"

namespace infra::repo {

    class AngleSourceFactory final {
    public:
        explicit AngleSourceFactory(
            const std::string& ini_path,
            angle::AngleSourcePorts ports);

        ~AngleSourceFactory();

        std::unique_ptr<domain::ports::IAngleSource> load(domain::common::SourceId id);

    private:
        std::string ini_path_;
        angle::AngleSourcePorts ports_;
    };

}

#endif // CLEANGRADUATOR_ANGLESOURCEFACTORY_H