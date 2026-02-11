#include "QtCameraGridSettingsRepository.h"

#include <QSettings>
#include <QString>

namespace infrastructure::settings {

    application::dto::VideoSourceGridSettings
    QtCameraGridSettingsRepository::loadGridSettings() const
    {
        if (cache_.has_value()) {
            return *cache_;
        }

        cache_ = loadFromFile();
        return *cache_;
    }


    application::dto::VideoSourceGridSettings
    QtCameraGridSettingsRepository::loadFromFile() const
    {
        QSettings settings(QString::fromStdString(path_), QSettings::IniFormat);

        application::dto::VideoSourceGridSettings result{};

        settings.beginGroup("CameraGrid");

        const auto gridString =
            settings.value("GridString", "").toString().toStdString();

        result.grid_string =
            application::dto::VideoSourceGridString(gridString);

        result.open_cameras_at_startup =
            settings.value("OpenCamerasAtStartup", false).toBool();

        settings.beginGroup("Crosshair");

        result.crosshair.visible =
            settings.value("Visible", false).toBool();

        result.crosshair.radius =
            settings.value("Radius", 0.1f).toFloat();

        settings.beginGroup("Color");

        result.crosshair.color.color1 =
            settings.value("Outside", 0xFFFFFFFF).toUInt();

        result.crosshair.color.color2 =
            settings.value("Inside", 0x000000FF).toUInt();

        settings.endGroup(); // Color
        settings.endGroup(); // Crosshair
        settings.endGroup(); // CameraGrid

        return result;
    }


    void QtCameraGridSettingsRepository::saveGridSettings(
        const application::dto::VideoSourceGridSettings& settingsData)
    {
        QSettings settings(QString::fromStdString(path_), QSettings::IniFormat);

        settings.beginGroup("CameraGrid");

        settings.setValue(
            "GridString",
            QString::fromStdString(settingsData.grid_string.toString())
        );

        settings.setValue(
            "OpenCamerasAtStartup",
            settingsData.open_cameras_at_startup
        );

        settings.beginGroup("Crosshair");

        settings.setValue("Visible", settingsData.crosshair.visible);
        settings.setValue("Radius", settingsData.crosshair.radius);

        settings.beginGroup("Color");

        settings.setValue("Outside", settingsData.crosshair.color.color1);
        settings.setValue("Inside", settingsData.crosshair.color.color2);

        settings.endGroup();
        settings.endGroup();
        settings.endGroup();

        settings.sync();

        // ---- Update cache ----
        cache_ = settingsData;
    }


} // namespace infrastructure::settings
