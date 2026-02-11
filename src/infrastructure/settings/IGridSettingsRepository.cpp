#include "IGridSettingsRepository.h"

#include <QSettings>
#include <QString>

using namespace application::dto;
using namespace infrastructure::settings;

IGridSettingsRepository::IGridSettingsRepository(std::string iniFilePath) {
}


VideoSourceGridSettings IGridSettingsRepository::load() const {
    if (cache_.has_value()) {
        return *cache_;
    }

    cache_ = loadFromFile();
    return *cache_;
}


VideoSourceGridSettings IGridSettingsRepository::loadFromFile() const {
    QSettings settings(QString::fromStdString(path_), QSettings::IniFormat);

    VideoSourceGridSettings result{};

    settings.beginGroup("CameraGrid");

    const auto gridString = settings.value("string", "").toString().toStdString();
    result.string = VideoSourceGridString::fromString(gridString);

    result.open_cameras_at_startup = settings.value("open_cameras_at_startup", false).toBool();

    settings.beginGroup("Crosshair");

    result.crosshair.visible =
        settings.value("visible", false).toBool();

    result.crosshair.radius =
        settings.value("radius", 0.1f).toFloat();

    settings.beginGroup("Color");

    result.crosshair.color.color1 =
        settings.value("color1", 0xFFFFFFFF).toUInt();

    result.crosshair.color.color2 =
        settings.value("color2", 0x000000FF).toUInt();

    settings.endGroup(); // Color
    settings.endGroup(); // Crosshair
    settings.endGroup(); // CameraGrid

    return result;
}

void IGridSettingsRepository::save(const VideoSourceGridSettings &settingsData) {

    QSettings settings(QString::fromStdString(path_), QSettings::IniFormat);

    settings.beginGroup("CameraGrid");

    settings.setValue(
        "string",
        QString::fromStdString(settingsData.string.asString())
    );

    settings.setValue(
        "open_cameras_at_startup",
        settingsData.open_cameras_at_startup
    );

    settings.beginGroup("Crosshair");

    settings.setValue("visible", settingsData.crosshair.visible);
    settings.setValue("radius", settingsData.crosshair.radius);

    settings.beginGroup("Color");

    settings.setValue("color1", settingsData.crosshair.color.color1);
    settings.setValue("color2", settingsData.crosshair.color.color2);

    settings.endGroup();
    settings.endGroup();
    settings.endGroup();

    settings.sync();

    // ---- Update cache ----
    cache_ = settingsData;
}

