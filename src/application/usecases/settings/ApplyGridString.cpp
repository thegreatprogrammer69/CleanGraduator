#include "ApplyGridString.h"
#include "application/ports/outbound/settings/IVideoSourceGridSettingsRepository.h"
#include "application/services/VideoSourceGridService.h"

using namespace application::usecases;

ApplyCameraGridSettings::ApplyCameraGridSettings(ApplyCameraGridSettingsPorts ports, ApplyCameraGridSettingsServices services)
    : logger_(ports.logger)
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
    const auto apply_string_result = grid_service_.applyCameraSettings(settings);
    result.correct_settings = apply_string_result.correct_settings;
    result.error = apply_string_result.error;

    if (result.error.empty()) {
        logger_.info("camera grid settings successfully applied");
    }
    else {
        logger_.warn("camera grid settings applied with errors");
    }

    return result;
}
