#include "CalibrationSessionController.h"


using namespace application::orchestrators;
using namespace domain::common;

CalibrationSessionController::CalibrationSessionController(
    CalibrationSessionControllerPorts ports,
    CalibrationProcessOrchestrator& runtime)
    : runtime_(runtime)
    , logger_(ports.logger)
    , settings_query_(ports.settings_query)
{
}


CalibrationSessionController::~CalibrationSessionController() = default;

bool CalibrationSessionController::isRunning() const
{
    return runtime_.isRunning();
}

void CalibrationSessionController::start(CalibrationSessionControllerInput inp)
{
    if (runtime_.isRunning()) {
        logger_.warn("CalibrationSessionController::start ignored: already running");
        return;
    }

    calibration_mode_ = inp.calibration_mode;

    const auto config = buildConfig();
    if (!config) {
        logger_.error("CalibrationSessionController::start failed: invalid configuration");
        return;
    }

    logger_.info("CalibrationSessionController: starting calibration session");
    runtime_.start(*config);
}

void CalibrationSessionController::stop()
{
    if (!runtime_.isRunning()) {
        logger_.warn("CalibrationSessionController::stop ignored: not running");
        return;
    }

    logger_.info("CalibrationSessionController: stop requested");
    runtime_.stop();
}

void CalibrationSessionController::abort()
{
    logger_.error("CalibrationSessionController: abort requested");
    runtime_.abort();
}

std::optional<CalibrationProcessOrchestratorInput> CalibrationSessionController::buildConfig()
{
    CalibrationProcessOrchestratorInput config;

    config.calibration_mode = calibration_mode_;
    config.angle_unit = AngleUnit::deg;

    const auto pressure_unit = settings_query_.currentPressureUnit();
    const auto pressure_points = settings_query_.currentGaugePressurePoints();

    if (!pressure_unit) {
        logger_.error("CalibrationSessionController: pressure unit not configured");
        return std::nullopt;
    }

    if (!pressure_points || pressure_points->value.empty()) {
        logger_.error("CalibrationSessionController: pressure points not configured");
        return std::nullopt;
    }

    config.pressure_unit = *pressure_unit;
    config.pressure_points = *pressure_points;

    return config;
}
