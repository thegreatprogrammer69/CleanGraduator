#include "CalibrationResultBuilder.h"

#include <exception>
#include <limits>

#include "domain/ports/calibration/calculation/ICalibrationCalculator.h"

using namespace application::orchestrators;
using namespace domain::common;



CalibrationResultBuilder::CalibrationResultBuilder(CalibrationResultBuilderPorts ports)
    : logger_(ports.logger), ports_(ports)
{
    ports_.calibration_recorder.addObserver(*this);
    logger_.info("CalibrationResultBuilder constructor called");
}

CalibrationResultBuilder::~CalibrationResultBuilder() {
    ports_.calibration_recorder.removeObserver(*this);
    logger_.info("CalibrationResultBuilder destructor called");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void CalibrationResultBuilder::onCalibrationRecorderEvent(const CalibrationRecorderEvent &ev) {
    std::visit(
    [this](const auto& e)
    {
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, CalibrationRecorderEvent::RecordingStarted> ||
            std::is_same_v<T, CalibrationRecorderEvent::SessionEnded> ||
            std::is_same_v<T, CalibrationRecorderEvent::RecordingStopped>)
        {
            handleEvent(e);
        }
    },
    ev.data);
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::RecordingStarted &e) {
    logger_.info("RecordingStarted: points_count={} sources_count={} directions_count={}",
        e.layout.points.size(), e.layout.sources.size(), e.layout.directions.size());

    active_layout_ = e.layout;
    active_result_ = CalibrationResult(e.layout, e.gauge, e.calibration_mode);

    observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
        o.onCalibrationResultUpdated(*active_result_);
    });
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::SessionEnded &e) {
    if (!active_result_ || !active_layout_) {
        logger_.warn("SessionEnded received but no active calibration session");
        return;
    }

    logger_.info(
        "SessionEnded: direction={}, pressure={}",
        e.id.direction,
        e.result.id.point.pressure
    );


    CalibrationCellKey key;
    key.direction = e.id.direction;
    key.point_id = e.id.point;

    for (const auto& source_id : active_layout_->sources) {
        // Оптимизируем поиск: ищем один раз
        auto it = e.result.angle_series.find(source_id);
        if (it == e.result.angle_series.end()) continue;

        key.source_id = source_id;

        logger_.info(
            "Processing camera {}: angle_samples={}, pressure_samples={}",
            source_id.value, it->second.size(), e.result.pressure_series.size());


        CalibrationCalculatorInput calculator_input {
            e.result.id.point.pressure,
            it->second, // Используем найденное значение
            e.result.pressure_series
        };

        try {
            const auto result = ports_.calibration_calculator.compute(calculator_input);
            active_result_->setCell(key, result.cell);

            const auto angle = result.cell.angle();
            logger_.info(
                "Calibration cell updated: source={}, direction={}, pressure={}, angle={}, issues_count={}",
                key.source_id.value,
                key.direction,
                key.point_id.pressure,
                angle ? *angle : std::numeric_limits<float>::quiet_NaN(),
                result.cell.issues().size()
            );

            observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
                o.onCalibrationResultUpdated(*active_result_);
            });
        } catch (const std::exception& ex) {
            logger_.error(
                "Failed to compute calibration cell: source={}, direction={}, point_id={}, error={}",
                key.source_id.value,
                key.direction,
                key.point_id.id,
                ex.what());
        }
    }
}

void CalibrationResultBuilder::handleEvent(const CalibrationRecorderEvent::RecordingStopped &e) {
    if (!active_result_) return;
    active_result_->markReady();
    observers_.notify([this] (domain::ports::ICalibrationResultObserver &o) {
        o.onCalibrationResultUpdated(*active_result_);
    });
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
