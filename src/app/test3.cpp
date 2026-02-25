#include <QApplication>
#include "ControlPanelWidget.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/motor/g540/as_lpt/G540LptMotorDriver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    infra::logging::ConsoleLogger logger;

    infra::motor::MotorDriverPorts ports {
        .logger = logger
    };

    infra::motors::G540LptMotorDriverConfig config;

    infra::motor::G540LptMotorDriver motor_driver(ports, config);

    return a.exec();
}
