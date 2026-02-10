#include "ApplyCameraGridSettings.h"
#include "application/ports/outbound/settings/ICameraGridSettingsRepository.h"
#include "application/services/VideoSourceGridService.h"

using namespace application::usecases;

ApplyCameraGridSettings::ApplyCameraGridSettings(ApplyCameraGridSettingsPorts ports, ApplyCameraGridSettingsServices services)
    : logger_(ports.logger)
    , settings_repo_(ports.settings_repo)
    , crosshair_listener_(ports.crosshair_listener)
    , grid_service_(services.grid_service)
{
    logger_.info("constructor called");
}

ApplyCameraGridSettings::~ApplyCameraGridSettings() {
    logger_.info("destructor called");
}

ApplyCameraGridSettings::ExecuteResult ApplyCameraGridSettings::execute(const dto::VideoSourceGridSettings &settings) {
    logger_.info("execute called");

    ExecuteResult result{};

    // Открываем камеры
    const auto apply_string_result = grid_service_.applyCameraString(settings.grid_string);
    result.correct_settings = settings;
    result.correct_settings.grid_string = apply_string_result.correct_string;
    result.error = apply_string_result.error;

    // Применяем настройки перекрестия
    crosshair_listener_.onCrosshairChanged(settings.crosshair);

    // Сохраняем настройки
    try {
        auto all = settings_repo_.load();
        all.camera_grid = result.correct_settings;
        settings_repo_.save(all);
    }
    catch(const std::exception& e) {
        result.error = e.what();
    }

    if (result.error.empty()) {
        logger_.info("camera grid settings successfully applied");
    }
    else {
        logger_.warn("camera grid settings applied with errors");
    }

    return result;
}
