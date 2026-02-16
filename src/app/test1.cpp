// #include <QApplication>
// #include <thread>
//
// #include "application/fmt/fmt_application.h"
// #include "domain/core/process/ProcessLifecycleState.h"
// #include "infrastructure/logging/ConsoleLogger.h"
//
// #include "infrastructure/motor/g540/as_lpt/G540LptMotorDriver.h"
// #include "infrastructure/motor/g540/as_lpt/G540LptMotorDriverConfig.h"
// #include "infrastructure/motor/g540/as_lpt/G540LptMotorDriverPorts.h"
//
// using namespace infra::motors;
// using namespace infra::logging;
//
// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);
//
//     ConsoleLogger logger;
//
//     G540LptMotorDriverConfig config;
//         config.bit_begin_limit_switch = 1;
//         config.bit_end_limit_switch = 2;
//         config.byte_close_both_flaps = 0;
//         config.byte_open_input_flap = 8;
//         config.byte_open_output_flap = 9;
//         config.lpt_port = 0x385;
//         config.max_freq_hz = 40;
//         config.min_freq_hz = 0;
//
//     G540LptMotorDriverPorts ports {
//         logger
//     };
//
//     G540LptMotorDriver g540(ports, config);
//     std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     g540.openInputFlap();
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     g540.closeFlaps();
//
//
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000000));
//
//     return app.exec();
// }