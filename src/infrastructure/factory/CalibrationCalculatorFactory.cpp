#include "CalibrationCalculatorFactory.h"

#include "infrastructure/calculation/calibration/LinearCalibrationCalculator.h"
#include "infrastructure/calculation/calibration/LinearCalibrationCalculatorConfig.h"
#include "infrastructure/utils/ini/IniFile.h"

namespace {

    utils::ini::IniFile loadIniOrThrow(const std::string& path) {
        utils::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error("CalibrationCalculatorFactory: failed to load setup: " + path);
        }
        return ini;
    }

}

infra::repo::CalibrationCalculatorFactory::CalibrationCalculatorFactory(
    const std::string& ini_path,
    calc::CalibrationCalculatorPorts ports)
    : ini_path_(ini_path),
      ports_(ports)
{
}

infra::repo::CalibrationCalculatorFactory::~CalibrationCalculatorFactory() = default;

std::unique_ptr<domain::ports::ICalibrationCalculator> infra::repo::CalibrationCalculatorFactory::load()
{
    auto ini = loadIniOrThrow(ini_path_);

    std::vector<std::unique_ptr<domain::ports::ICalibrationCalculator>> result;
        const auto section_name = "calibrator";

        if (!ini.hasSection(section_name))
            throw std::runtime_error("CalibrationCalculatorFactory: no section found");

        const auto section = ini[section_name];
        const std::string algorithm = section.getString("algorithm", "");

        if (algorithm == "linear") {
            calc::LinearCalibrationCalculatorConfig config{};

            // Пример чтения параметров (добавь реальные поля конфига)
            config.min_angle_values_in_series = section.getInt("min_angle_values_in_series", config.min_angle_values_in_series);
            config.min_pressure_values_in_series = section.getInt("min_pressure_values_in_series", config.min_pressure_values_in_series);
            config.overlap = section.getInt("overlap", config.overlap);

            return std::make_unique<calc::LinearCalibrationCalculator>(ports_, config);
        }

        throw std::runtime_error("unknown calibration algorithm: " + algorithm);
}
