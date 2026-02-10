#include "QtCameraGridSettingsRepository.h"

#include <QSettings>
#include <QString>

namespace infrastructure::settings {

QtCameraGridSettingsRepository::QtCameraGridSettingsRepository(std::string iniFilePath)
    : path_(std::move(iniFilePath))
{
}

application::dto::UserSettings QtCameraGridSettingsRepository::load() const
{
    QSettings settings(QString::fromStdString(path_), QSettings::IniFormat);

    application::dto::UserSettings result{};

    // CameraGrid
    settings.beginGroup("CameraGrid");

    // VideoSourceGridString
    {
        const auto gridString =
            settings.value("GridString", "").toString().toStdString();
        result.camera_grid.grid_string =
            application::dto::VideoSourceGridString(gridString);
    }

    // Open cameras at startup
    result.camera_grid.open_cameras_at_startup =
        settings.value("OpenCamerasAtStartup", false).toBool();

    // Crosshair
    settings.beginGroup("Crosshair");

    result.camera_grid.crosshair.visible =
        settings.value("Visible", false).toBool();

    result.camera_grid.crosshair.radius =
        settings.value("Radius", 0.1f).toFloat();

    // Crosshair colors
    settings.beginGroup("Color");

    result.camera_grid.crosshair.color.color1 =
        settings.value("Outside", 0xFFFFFFFF).toUInt();

    result.camera_grid.crosshair.color.color2 =
        settings.value("Inside", 0x000000FF).toUInt();

    settings.endGroup(); // Color
    settings.endGroup(); // Crosshair
    settings.endGroup(); // CameraGrid

    return result;
}

void QtCameraGridSettingsRepository::save(const application::dto::UserSettings& settingsData)
{
    QSettings settings(QString::fromStdString(path_), QSettings::IniFormat);

    // CameraGrid
    settings.beginGroup("CameraGrid");

    settings.setValue(
        "GridString",
        QString::fromStdString(
            settingsData.camera_grid.grid_string.toString()
        )
    );

    settings.setValue(
        "OpenCamerasAtStartup",
        settingsData.camera_grid.open_cameras_at_startup
    );

    // Crosshair
    settings.beginGroup("Crosshair");

    settings.setValue(
        "Visible",
        settingsData.camera_grid.crosshair.visible
    );

    settings.setValue(
        "Radius",
        settingsData.camera_grid.crosshair.radius
    );

    // Crosshair colors
    settings.beginGroup("Color");

    settings.setValue(
        "Outside",
        settingsData.camera_grid.crosshair.color.color1
    );

    settings.setValue(
        "Inside",
        settingsData.camera_grid.crosshair.color.color2
    );

    settings.endGroup(); // Color
    settings.endGroup(); // Crosshair
    settings.endGroup(); // CameraGrid

    settings.sync();
}

} // namespace infrastructure::settings
