#include "CalibrationSessionControl.h"

using namespace application::usecase;

bool CalibrationSessionControl::isRunning() const
{
    return orchestrator_.isRunning();
}

bool CalibrationSessionControl::start(
    domain::common::CalibrationMode mode,
    const bool slowdown_at_checkpoints,
    const bool play_valve,
    std::string& error_text)
{
    error_text.clear();

    const auto calibration_context = loadCalibrationContext(error_text);
    if (!calibration_context)
        return false;

    if (!calibration_context->gauge.points.isCorrect()) {
        error_text =
            "Не удалось запустить: проверьте настройки шкалы и единиц давления.";
        return false;
    }

    application::orchestrators::CalibrationOrchestratorInput input{
        mode,
        calibration_context->pressure_unit.unit,
        domain::common::AngleUnit::deg,
        calibration_context->gauge,
        slowdown_at_checkpoints,
        play_valve
    };

    if (!orchestrator_.start(input)) {
        error_text = "Не удалось запустить процесс калибровки.";
        return false;
    }

    return true;
}

void CalibrationSessionControl::stop()
{
    orchestrator_.stop();
}

void CalibrationSessionControl::emergencyStop()
{
    orchestrator_.stop();
}

void CalibrationSessionControl::addObserver(
    application::ports::CalibrationOrchestratorObserver& observer)
{
    orchestrator_.addObserver(observer);
}

void CalibrationSessionControl::removeObserver(
    application::ports::CalibrationOrchestratorObserver& observer)
{
    orchestrator_.removeObserver(observer);
}

std::optional<application::models::CalibrationContext>
CalibrationSessionControl::loadCalibrationContext(std::string& error_text)
{
    const auto calibration_context = context_provider_.current();
    if (!calibration_context) {
        error_text =
            "Не удалось запустить: проверьте настройки шкалы, манометра, единиц давления и расположения.";
        return std::nullopt;
    }

    return calibration_context;
}
