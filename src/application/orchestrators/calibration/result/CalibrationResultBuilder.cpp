#include "CalibrationResultBuilder.h"

#include "domain/ports/calibration/calculation/ICalibrationCalculator.h"



using namespace application::orchestrators;
using namespace domain::common;

// TODO ёпта, надо улучшить логирование

CalibrationResultBuilder::Cali brationResultBuilder(CalibrationResultBuilderPorts ports)
    : logger_(ports.logger), ports_(ports)
{
    logger_.info("CalibrationResultBuilder constructor called");
}

CalibrationResultBuilder::~CalibrationResultBuilder() {
    logger_.info("CalibrationResultBuilder destructor called");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CalibrationResultBuilder::onCalibrationRecorderEvent(const CalibrationRecorderEvent &ev) {
    std::visit(
    [this](const auto& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, CalibrationRecorderEvent::RecordingStarted> ||
            std::is_same_v<T, CalibrationRecorderEvent::SessionEnded>)
        {
            handleEvent(e);
        }
    },
    ev.data);
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::RecordingStarted &e) {
    logger_.info("Handling RecordingStarted event");
    active_layout_ = e.layout;
    active_result_ = CalibrationResult(e.layout);
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::SessionEnded &e) {
    if (!active_result_ || !active_layout_) return;

    CalibrationCellKey key;
    key.direction = e.id.direction;
    key.point_id = e.id.point;

    for (const auto& source_id : active_layout_->sources) {
        // Оптимизируем поиск: ищем один раз
        auto it = e.result.angle_series.find(source_id);
        if (it == e.result.angle_series.end()) continue;

        key.source_id = source_id;

        CalibrationCalculatorInput calculator_input {
            e.result.id.point.pressure,
            it->second, // Используем найденное значение
            e.result.pressure_series
        };

        const auto result = ports_.calibration_calculator.compute(calculator_input);
        active_result_->setCell(key, result.cell);

        observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
            o.onCalibrationResultUpdated(*active_result_);
        });
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::optional<CalibrationResult>& CalibrationResultBuilder::currentResult() const {
    return active_result_;
}

void CalibrationResultBuilder::addObserver(domain::ports::ICalibrationResultObserver &o) {
    observers_.add(o);
}

void CalibrationResultBuilder::removeObserver(domain::ports::ICalibrationResultObserver &o) {
    observers_.remove(o);
}

