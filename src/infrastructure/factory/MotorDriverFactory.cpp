#include "MotorDriverFactory.h"
#include <stdexcept>

#include "infrastructure/motor/g540/as_lpt/G540LptMotorDriver.h"
#include "infrastructure/motor/g540/as_lpt/G540LptMotorDriverConfig.h"
#include "infrastructure/utils/ini/IniFile.h"

using namespace infra::repo;
using namespace infra::motor;

namespace {

    utils::ini::IniFile loadIniOrThrow(const std::string& path) {
        utils::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error("CalibrationCalculatorFactory: failed to load setup: " + path);
        }
        return ini;
    }

}

MotorDriverFactory::MotorDriverFactory(const std::string &ini_path, MotorDriverPorts ports)
    : ini_path_(ini_path), ports_(ports)
{
}

MotorDriverFactory::~MotorDriverFactory() {
}

std::unique_ptr<domain::ports::IMotorDriver> MotorDriverFactory::load() {
    auto ini = loadIniOrThrow(ini_path_);

    std::unique_ptr<domain::ports::IMotorDriver> result;
    const auto section_name = "motor";

    if (!ini.hasSection(section_name))
        throw std::runtime_error("MotorDriverFactory: no section found");

    const auto section = ini[section_name];
    const std::string implementation = section.getString("implementation", "");

    if (implementation == "g540lpt") {
        motors::G540LptMotorDriverConfig config;

        // Пример чтения параметров (добавь реальные поля конфига)
        config.bit_begin_limit_switch = section.getInt("bit_begin_limit_switch", config.bit_begin_limit_switch);
        config.bit_end_limit_switch = section.getInt("bit_end_limit_switch", config.bit_end_limit_switch);
        config.byte_close_both_flaps = section.getHex("byte_close_both_flaps", config.byte_close_both_flaps);
        config.byte_open_input_flap = section.getHex("byte_open_input_flap", config.byte_open_input_flap);
        config.byte_open_output_flap = section.getHex("byte_open_output_flap", config.byte_open_output_flap);
        config.lpt_port = section.getHex("lpt_port", config.lpt_port);
        config.max_freq_hz = section.getInt("max_freq_hz", config.max_freq_hz);
        config.min_freq_hz = section.getInt("min_freq_hz", config.min_freq_hz);

        auto motor_driver = std::make_unique<G540LptMotorDriver>(ports_, config);
        return motor_driver;
    }

    throw std::runtime_error("unknown Motor Driver implementation: " + implementation);
}
