// #include <QApplication>
// #include <thread>
//
// #include "application/fmt/fmt_application.h"
// #include "infrastructure/logging/ConsoleLogger.h"
// #include "infrastructure/motion/g540/as_lpt/G540LptMotorDriver.h"
// #include "infrastructure/motion/g540/as_lpt/G540LptMotorDriverConfig.h"
//
//
// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);
//
//     infra::logging::ConsoleLogger logger;
//
//     infra::motors::G540LptMotorDriverConfig config;
//         config.bit_begin_limit_switch = 1;
//         config.bit_end_limit_switch = 2;
//         config.byte_close_both_flaps = 0;
//         config.byte_open_input_flap = 8;
//         config.byte_open_output_flap = 9;
//         config.lpt_port = 0x385;
//         config.max_freq_hz = 40;
//         config.min_freq_hz = 0;
//
//     infra::motors::MotorDriverPorts ports {
//         logger
//     };
//
//     infra::motors::G540LptMotorDriver g540(ports, config);
//     g540.initialize();
//
//     std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     g540.openInputFlap();
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     g540.closeFlaps();
//
//
//     g540.start();
//     g540.stop();
//
//     std::this_thread::sleep_for(std::chrono::milliseconds(1000000));
//
//     return app.exec();
// }
