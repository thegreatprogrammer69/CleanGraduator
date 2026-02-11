#include "DomainModule.h"

#include <stdexcept>

#include "application/interactors/AngleFromVideoInteractor.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/calculation/angle/CastAnglemeterConfig.h"
#include "infrastructure/utils/ini/IniFile.h"

using utils::ini::IniFile;

DomainModule::AnglemeterConfig DomainModule::loadAnglemeterConfig(const std::string& configDirectory) {
    IniFile ini;
    const auto path = configDirectory + "/anglemeter.ini";
    if (!ini.load(path)) {
        throw std::runtime_error("Failed to load config: " + path);
    }

    AnglemeterConfig config;
    const auto& section = ini["anglemeter"];
    config.brightLim = section.getInt("bright_lim", config.brightLim);
    config.maxPairs = section.getInt("max_pairs", config.maxPairs);
    config.scanStep = section.getInt("scan_step", config.scanStep);
    return config;
}

std::unique_ptr<domain::ports::IAngleCalculator> DomainModule::createAnglemeter(
    domain::ports::ILogger& logger,
    const AnglemeterConfig& config)
{
    infra::calculation::AnglemeterPorts ports{.logger = logger};

    CastAnglemeterConfig castConfig{};
    castConfig.bright_lim = config.brightLim;
    castConfig.max_pairs = config.maxPairs;
    castConfig.scan_step = config.scanStep;

    return std::make_unique<infra::calculation::CastAnglemeter>(ports, castConfig);
}

std::unique_ptr<application::interactors::AngleFromVideoInteractor> DomainModule::createAngleFromVideoInteractor(
    domain::ports::ILogger& logger,
    domain::ports::IAngleCalculator& calculator)
{
    application::interactors::AngleFromVideoInteractorPorts ports{
        .logger = logger,
        .anglemeter = calculator,
    };
    return std::make_unique<application::interactors::AngleFromVideoInteractor>(ports);
}
