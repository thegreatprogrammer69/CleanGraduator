//
// Created by mint on 17.02.2026.
//

#ifndef CLEANGRADUATOR_MOTORDRIVERFACTORY_H
#define CLEANGRADUATOR_MOTORDRIVERFACTORY_H
#include <memory>
#include <string>

#include "../../domain/ports/drivers/valve/IValveDriver.h"
#include "../../domain/ports/drivers/motor/IMotorDriver.h"
#include "infrastructure/motion/g540/MotorDriverPorts.h"

namespace infra::repo {
    class MotorDriverFactory {
    public:
        MotorDriverFactory(const std::string& ini_path, motors::MotorDriverPorts ports);
        ~MotorDriverFactory();

        std::unique_ptr<domain::ports::IMotorDriver> load();
        domain::ports::IValveDriver* load_valve_driver();
    private:
        std::string ini_path_;
        motors::MotorDriverPorts ports_;
        domain::ports::IValveDriver* valve_driver_{nullptr};
    };
}


#endif //CLEANGRADUATOR_MOTORDRIVERFACTORY_H