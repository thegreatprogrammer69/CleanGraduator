#include "PressureSourceFactory.h"

#include "infrastructure/pressure/dm5002/DM5002PressureSensor.h"
#include "infrastructure/pressure/dm5002/DM5002PressureSensorConfig.h"
#include "infrastructure/pressure/fake/FakePressureSource.h"
#include "infrastructure/utils/ini/IniFile.h"

namespace {

    utils::ini::IniFile loadIniOrThrow(const std::string& path) {
        utils::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error("PressureSourceFactory: failed to load setup: " + path);
        }
        return ini;
    }

}

namespace infra::repo {

    PressureSourceFactory::PressureSourceFactory(
            const std::string& ini_path,
            pressure::PressureSourcePorts ports)
        : ini_path_(ini_path)
        , ports_(ports)
    {
    }

    PressureSourceFactory::~PressureSourceFactory() = default;

    std::unique_ptr<domain::ports::IPressureSource>
    PressureSourceFactory::load()
    {
        auto ini = loadIniOrThrow(ini_path_);

        const std::string section_name = "pressure_source";

        if (!ini.hasSection(section_name)) {
            throw std::runtime_error(
                "PressureSourceFactory: no section 'pressure_source'");
        }

        auto section = ini[section_name];

        const std::string type = section.getString("type", "");

        if (type == "dm5002") {
            pressure::DM5002PressureSensorConfig config{};

            config.com_port = section.getString("com_port", config.com_port);
            config.poll_rate = section.getInt("poll_rate", config.poll_rate);

            // --- protocol parsing
            const std::string protocolStr =
                section.getString("protocol", "standard");

            if (protocolStr == "standard") {
                config.protocol =
                    pressure::DM5002PressureSensorConfig::Protocol::Standard;
            }
            else if (protocolStr == "rf") {
                config.protocol =
                    pressure::DM5002PressureSensorConfig::Protocol::RF;
            }
            else {
                throw std::runtime_error(
                    "DM5002: unknown protocol: " + protocolStr +
                    " (expected 'standard' or 'rf')");
            }

            if (config.com_port.empty()) {
                throw std::runtime_error("DM5002: com_port is empty");
            }

            if (config.poll_rate <= 0) {
                throw std::runtime_error("DM5002: poll_rate must be > 0");
            }

            return std::make_unique<pressure::DM5002PressureSensor>(
                ports_,
                config
            );
        }

        if (type == "fake") {

            pressure::FakePressureSourceConfig config{};

            const double fromPa = section.getFloat("from_kgf", 0.0);
            const double toPa   = section.getFloat("to_kgf", 100.0);

            config.from = domain::common::Pressure::fromKgfCm2(fromPa);
            config.to   = domain::common::Pressure::fromKgfCm2(toPa);

            config.duration = std::chrono::milliseconds(
                section.getInt("duration_ms", 5000));

            config.poll_interval = std::chrono::milliseconds(
                section.getInt("poll_interval_ms", 50));

            const std::string modeStr =
                section.getString("mode", "ramp");

            if (modeStr == "ramp") {
                config.mode = pressure::FakePressureSourceConfig::Mode::Ramp;
            }
            else if (modeStr == "pingpong") {
                config.mode = pressure::FakePressureSourceConfig::Mode::PingPong;
            }
            else {
                throw std::runtime_error(
                    "FakePressureSource: unknown mode: " + modeStr);
            }

            return std::make_unique<pressure::FakePressureSource>(
                ports_,
                config
            );
        }

        throw std::runtime_error(
            "PressureSourceFactory: unknown pressure source type: " + type);
    }

}
