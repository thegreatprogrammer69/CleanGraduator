#include "AngleSourceFactory.h"

#include "infrastructure/angle/fake/FakeAngleSourceFromVideo.h"
#include "infrastructure/angle/from_video/AngleSourceFromVideo.h"
#include "infrastructure/angle/from_video/AngleSourceFromVideoConfig.h"
#include "shared/ini/IniFile.h"
#include <cmath>

namespace {

    shared::ini::IniFile loadIniOrThrow(const std::string& path) {
        shared::ini::IniFile ini;
        if (!ini.load(path)) {
            throw std::runtime_error(
                "AngleSourceFactory: failed to load setup: " + path);
        }
        return ini;
    }

}

namespace infra::repo {

AngleSourceFactory::AngleSourceFactory(
        const std::string& ini_path,
        angle::AngleSourcePorts ports)
    : ini_path_(ini_path)
    , ports_(ports)
{
}

AngleSourceFactory::~AngleSourceFactory() = default;

std::unique_ptr<domain::ports::IAngleSource>
AngleSourceFactory::load(domain::common::SourceId id)
{
    auto ini = loadIniOrThrow(ini_path_);

    const std::string section_name = "angle_source";

    if (!ini.hasSection(section_name)) {
        throw std::runtime_error(
            "AngleSourceFactory: no section 'angle_source'");
    }

    auto section = ini[section_name];

    const std::string type = section.getString("type", "");

    // ============================================================
    // REAL VIDEO-BASED SOURCE
    // ============================================================

    if (type == "video") {

        angle::AngleSourceFromVideoConfig config{};
        // если появятся параметры — парсятся здесь

        return std::make_unique<angle::AngleSourceFromVideo>(
            id,
            ports_,
            config
        );
    }

    // ============================================================
    // FAKE SOURCE
    // ============================================================

    if (type == "fake") {

        angle::FakeAngleSourceFromVideoConfig config{};

        config.from_deg = section.getFloat("from_deg", 0.0);
        config.to_deg   = section.getFloat("to_deg", 360.0);

        config.duration_ms =
            static_cast<std::uint64_t>(
                section.getInt("duration_ms", 10000));
        config.nonlinearity_ratio = section.getFloat("nolin", 0.0);

        if (config.duration_ms == 0) {
            throw std::runtime_error(
                "Фейковый датчик угла: duration_ms должен быть больше 0");
        }

        if (config.from_deg == config.to_deg) {
            throw std::runtime_error(
                "Фейковый датчик угла: from_deg не должен быть равен to_deg");
        }

        if (!std::isfinite(config.nonlinearity_ratio)
            || config.nonlinearity_ratio < 0.0
            || config.nonlinearity_ratio >= 1.0) {
            throw std::runtime_error(
                "Фейковый датчик угла: nolin должен быть в диапазоне [0.0, 1.0)");
        }

        return std::make_unique<angle::FakeAngleSourceFromVideo>(
            id,
            ports_,
            config
        );
    }

    // ============================================================

    throw std::runtime_error(
        "AngleSourceFactory: unknown angle source type: " + type);
}

} // namespace infra::repo
