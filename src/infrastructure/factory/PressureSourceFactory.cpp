#include "PressureSourceFactory.h"

#include "infrastructure/pressure/dm5002/DM5002RFPressureSensor.h"
#include "infrastructure/pressure/dm5002/DM5002RFConfig.h"
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
            throw std::runtime_error("PressureSourceFactory: no section 'pressure_source'");
        }

        auto section = ini[section_name];

        const std::string type = section.getString("type", "");

        if (type == "dm5002" || type == "dm5002rf") {
            pressure::DM5002RFConfig config{};
            config.com_port = section.getString("com_port", config.com_port);
            config.poll_rate = section.getInt("poll_rate", config.poll_rate);

            if (config.com_port.empty()) {
                throw std::runtime_error("DM5002: com_port is empty");
            }

            return std::make_unique<infra::pressure::DM5002RFPressureSensor>(
                ports_,
                config
            );
        }

        throw std::runtime_error(
            "PressureSourceFactory: unknown pressure source type: " + type);
    }

}
