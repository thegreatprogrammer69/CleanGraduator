#include "AngleCalculatorFactory.h"

#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/calculation/angle/CastAnglemeterConfig.h"
#include "infrastructure/utils/ini/IniFile.h"

namespace {
    utils::ini::IniFile loadIniOrThrow(const std::string& path) {
        utils::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error("VideoSourceFactory: failed to load setup: " + path);
        }
        return ini;
    }
}

infra::repo::AngleCalculatorFactory::AngleCalculatorFactory(const std::string& ini_path, calc::AnglemeterPorts ports)
    : ini_path_(ini_path), ports_(ports)
{
}


infra::repo::AngleCalculatorFactory::~AngleCalculatorFactory() {
}


std::unique_ptr<domain::ports::IAngleCalculator> infra::repo::AngleCalculatorFactory::load() {
    auto ini = loadIniOrThrow(ini_path_);

        const auto section_name = "anglemeter";
        auto section = ini[section_name];

        if (!ini.hasSection(section_name)) {
            throw std::runtime_error("has no section anglemeter");
        }

        std::string algorithm = section.getString("algorithm", "");

        if (algorithm == "cast") {
            calc::CastAnglemeterConfig config{};
            config.bright_lim = section.getInt("bright_lim", config.bright_lim);
            config.max_pairs = section.getInt("max_pairs", config.max_pairs);
            config.scan_step = section.getInt("scan_step", config.scan_step);
            return std::make_unique<calc::CastAnglemeter>(ports_, config);
        }

        throw std::runtime_error("unknown anglemeter algorithm: " + algorithm);
}
