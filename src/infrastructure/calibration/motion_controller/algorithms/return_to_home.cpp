#include "return_to_home.h"

#include "domain/ports/logging/ILogger.h"

bool infra::calib::detail::executeHoming(
    domain::ports::ILogger &logger,
    domain::ports::IMotorDriver &motor_driver,
    domain::ports::IValveDriver &valve_driver) {

    logger.info("");
    return ;
}
