//
// Created by mint on 17.02.2026.
//

#ifndef CLEANGRADUATOR_MOTORDRIVERFACTORY_H
#define CLEANGRADUATOR_MOTORDRIVERFACTORY_H
#include <memory>
#include <string>

#include "domain/ports/drivers/motor/IMotorDriver.h"
#include "infrastructure/motor/MotorDriverPorts.h"

namespace infra::repo {
    class MotorDriverFactory {
    public:
        MotorDriverFactory(const std::string& ini_path, motor::MotorDriverPorts ports);
        ~MotorDriverFactory();

        std::unique_ptr<domain::ports::IMotorDriver> load();
    private:
        std::string ini_path_;
        motor::MotorDriverPorts ports_;
    };
}


#endif //CLEANGRADUATOR_MOTORDRIVERFACTORY_H