#include "VideoSourceOverlayFactory.h"

#include "infrastructure/utils/ini/IniFile.h"

#include <stdexcept>

#include "infrastructure/overlay/crosshair/CrosshairVideoOverlay.h"

namespace {

utils::ini::IniFile loadIniOrThrow(const std::string& path)
{
    utils::ini::IniFile ini;
    if (!ini.load(path))
        throw std::runtime_error(
            "VideoSourceOverlayFactory: failed to load config: " + path);

    return ini;
}

std::uint32_t parseColor(const std::string& value, std::uint32_t fallback)
{
    if (value.empty())
        return fallback;

    // ожидаем формат 0xRRGGBBAA
    return static_cast<std::uint32_t>(
        std::stoul(value, nullptr, 16)
    );
}

} // namespace

infra::repo::VideoSourceOverlayFactory::VideoSourceOverlayFactory(
    const std::string& ini_path,
    overlay::VideoOverlayPorts ports)
    : ini_path_(ini_path)
    , ports_(ports)
{
}

infra::repo::VideoSourceOverlayFactory::~VideoSourceOverlayFactory() = default;

std::vector<std::unique_ptr<domain::ports::IVideoSourceOverlay>>
infra::repo::VideoSourceOverlayFactory::load()
{
    auto ini = loadIniOrThrow(ini_path_);

    std::vector<std::unique_ptr<domain::ports::IVideoSourceOverlay>> result;

    decltype(auto) sections = ini.sections(); // <- требуется метод

    for (const auto &[section_name, section] : sections)
    {
        if (section_name.rfind("overlay_", 0) != 0)
            continue;

        std::string type = section.getString("type", "");

        if (type == "crosshair")
        {
            overlay::CrosshairVideoOverlayConfig config{};

            config.visible =
                section.getBool("visible", config.visible);

            config.radius =
                section.getFloat("radius", config.radius);

            config.color1 =
                static_cast<std::uint32_t>(
                    section.getInt("color1", config.color1));

            config.color2 =
                static_cast<std::uint32_t>(
                    section.getInt("color2", config.color2));

            result.push_back(
                std::make_unique<overlay::CrosshairVideoOverlay>(config)
            );

            continue;
        }

        throw std::runtime_error("unknown overlay type: " + type);
    }

    return result;
}
