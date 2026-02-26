#include "G540LptMotorDriver.h"
#include "domain/core/drivers/motor/MotorDriverEvent.h"
#include "infrastructure/platform/sleep/sleep.h"

namespace {
    using namespace domain::common;
    void genStepBytes(std::uint8_t (&out)[2], MotorDirection dir) {
        constexpr int axis = 0; // Ось X
        constexpr int shift = 2 * axis; // 0, 2, 4, 6 для X,Y,Z,A
        switch (dir) {
            case MotorDirection::Forward:
                out[0] = 0 << shift;
                out[1] = 1 << shift;
                break;
            case MotorDirection::Backward:
                out[0] = 2 << shift;
                out[1] = 3 << shift;
                break;
            default:
                out[0] = 0 << shift;
                out[1] = 0 << shift;
                break;
        }

    }
}

namespace infra::motor {
    using namespace domain::common;

    G540LptMotorDriver::G540LptMotorDriver(const MotorDriverPorts ports, const motors::G540LptMotorDriverConfig &config)
        : logger_(ports.logger), config_(config)
        , thread_worker_([this] () { this->loopOnce(); })
        , state_(MotorDriverState::Uninitialized)
        , frequency_(MotorFrequency(0))
        , direction_(MotorDirection::Forward)
    {
    }

    G540LptMotorDriver::~G540LptMotorDriver() {
        stop();
    }

    bool G540LptMotorDriver::initialize() {
        logger_.info("Initializing G540LptMotorDriver on LPT port {}", config_.lpt_port);

        try {
            lpt_port_.open(config_.lpt_port);
        }
        catch (const std::exception& e) {
            logger_.error("Failed to open LPT port {}: {}", config_.lpt_port, e.what());
            return false;
        }

        state_ = MotorDriverState::Stopped;

        logger_.info("G540LptMotorDriver successfully initialized. State set to Stopped");

        return true;
    }

    bool G540LptMotorDriver::start() {

        // 1. Инициализация при необходимости
        if (state_ == MotorDriverState::Uninitialized) {
            logger_.info("Driver uninitialized. Performing initialization.");
            if (!initialize()) {
                logger_.error("Initialization failed");
                return false;
            }
        }

        // 2. Уже запущен
        if (state_ == MotorDriverState::Running) {
            logger_.warn("Driver already running");
            return true;
        }

        // 3. Переход Stopped -> Running
        logger_.info("Starting motor driver");

        // 4. Сброс ошибок
        resetError();

        // 5. Проверка открытия порта
        if (!lpt_port_.isOpen()) {
            logger_.info("Opening LPT port {}", config_.lpt_port);
            lpt_port_.open(config_.lpt_port);
        }

        // 6. Запуск worker thread
        if (!thread_worker_.isRunning()) {
            logger_.info("Starting worker thread");
            thread_worker_.start();
        } else {
            logger_.info("Resuming worker thread");
            thread_worker_.resume();
        }

        // 7. Обновление состояния
        state_ = MotorDriverState::Running;

        return true;
    }

    void G540LptMotorDriver::stop() {

        // 1. Предупреждение при повторной остановке
        if (state_ == MotorDriverState::Stopped) {
            logger_.warn("Stop called while already stopped");
        }

        // 2. Пауза worker thread
        thread_worker_.pause();

        // 3. Обновление состояния
        state_ = MotorDriverState::Stopped;

        logger_.info("Motor driver stopped");
    }

    void G540LptMotorDriver::emergencyStop()
    {
        // 1. Немедленная остановка worker-потока
        thread_worker_.stop();

        // 2. Обновление состояние
        state_ = MotorDriverState::Stopped;

        // 4. Логирование аварийной остановки
        logger_.error(
            "!!! Emergency stop triggered. Driver state transitioned to Stopped. !!!"
        );

        // 5. Фиксируем ошибку
        MotorDriverError err;
        err.message = "Emergency stop triggered";
        error_.store(err);

        MotorDriverEvent::Fault ev;
        ev.error = err;
        notifier_.notifyEvent(MotorDriverEvent(ev));
    }

    MotorDriverState G540LptMotorDriver::state() const {
        return state_;
    }

    void G540LptMotorDriver::resetError() {
        error_.update([](MotorDriverError& err) { err.reset(); });
    }

    MotorDriverError G540LptMotorDriver::error() const {
        return error_.load();
    }

    void G540LptMotorDriver::enableWatchdog(std::chrono::milliseconds timeout) {
        software_watchdog_.start(timeout);
    }

    void G540LptMotorDriver::disableWatchdog() {
        software_watchdog_.stop();
    }

    void G540LptMotorDriver::setFrequency(const MotorFrequency frequency) {
        frequency_ = frequency;
        frequency_.clampTo(config_.max_freq_hz);
    }

    MotorFrequency G540LptMotorDriver::frequency() const {
        return frequency_;
    }

    MotorFrequencyLimits G540LptMotorDriver::frequencyLimits() const {
        MotorFrequencyLimits frequency_limits;
        frequency_limits.maxHz = config_.max_freq_hz;
        frequency_limits.minHz = config_.min_freq_hz;
        return frequency_limits;
    }

    void G540LptMotorDriver::setDirection(const MotorDirection dir) {
        direction_ = dir;
    }

    MotorDirection G540LptMotorDriver::direction() const {
        return direction_;
    }

    void G540LptMotorDriver::setFlapsState(MotorFlapsState state)
    {
        // 1. Атомарная замена состояния
        const auto previous = flaps_state_.exchange(state);


        switch (state) {
            case MotorFlapsState::ExhaustOpened:
                lpt_port_.write(2, config_.byte_open_output_flap);
                break;

            case MotorFlapsState::IntakeOpened:
                lpt_port_.write(2, config_.byte_open_input_flap);
                break;

            case MotorFlapsState::FlapsClosed:
                lpt_port_.write(2, config_.byte_close_both_flaps);
                break;

            default:
                logger_.error("Invalid flaps state requested");
                return;
        }

        // 4. Логирование изменения
        logger_.info(
            "Flaps state changed: {} -> {}",
            static_cast<int>(previous),
            static_cast<int>(state)
        );

        // 5. Генерация события
        MotorDriverEvent::FlapsStateChanged ev;
        ev.state = state;
        notifier_.notifyEvent(MotorDriverEvent(ev));
    }

    MotorFlapsState G540LptMotorDriver::flapsState() const {
        return flaps_state_;
    }

    MotorLimitsState G540LptMotorDriver::limits() const {
        const auto state = readState();
        MotorLimitsState limits_state;
        limits_state.home = (state & config_.bit_begin_limit_switch) != 0;
        limits_state.end  = (state & config_.bit_end_limit_switch) != 0;
        return limits_state;
    }

    void G540LptMotorDriver::addObserver(domain::ports::IMotorDriverObserver &o) {
        notifier_.addObserver(o);
    }

    void G540LptMotorDriver::removeObserver(domain::ports::IMotorDriverObserver &o) {
        notifier_.removeObserver(o);
    }

    void G540LptMotorDriver::loopOnce() {
        // 1. Чтение текущего состояния лимитных выключателей
        const auto current_limits = this->limits();

        // 2. Обработка изменения состояния лимитов (генерация LimitsChanged)
        handleLimitEvents(current_limits);

        // 3. Проверка безопасности (watchdog + активный лимит по направлению)
        if (!pollSafety(current_limits)) {
            logger_.warn("Safety condition violated. Stopping motor.");
            stop();
            return;
        }

        // 4. Генерация одного шага
        stepOnce();
    }

    bool G540LptMotorDriver::pollSafety(const MotorLimitsState &current_limits) {

        // 1. Проверка software watchdog
        if (software_watchdog_.expired()) {
            logger_.error("Watchdog timeout detected");

            MotorDriverError new_err;
            new_err.message = "Watchdog timeout";
            error_.store(new_err);

            // 2. Уведомление о fault
            MotorDriverEvent::Fault ev;
            ev.error = new_err;
            notifier_.notifyEvent(MotorDriverEvent(ev));

            return false;
        }

        // 3. Проверка достижения END при движении Forward
        if (direction_ == MotorDirection::Forward && current_limits.end) {
            logger_.info("END limit reached while moving Forward");

            MotorDriverEvent::StoppedAtEnd ev;
            notifier_.notifyEvent(MotorDriverEvent(ev));

            return false;
        }

        // 4. Проверка достижения HOME при движении Backward
        if (direction_ == MotorDirection::Backward && current_limits.home) {
            logger_.info("HOME limit reached while moving Backward");

            MotorDriverEvent::StoppedAtHome ev;
            notifier_.notifyEvent(MotorDriverEvent(ev));

            return false;
        }

        // 5. Все проверки пройдены
        return true;
    }

    void G540LptMotorDriver::stepOnce() {
        using namespace std::chrono;

        if (!frequency_.isValid()) platform::sleep(1ms);

        // 1. Формирование управляющих байтов для выбранного направления
        std::uint8_t step_bytes[2];
        genStepBytes(step_bytes, direction_);

        // 2. Расчёт половины периода шага
        const auto half_period = frequency_.halfPeriod();

        // 3. Первый фронт
        lpt_port_.write(step_bytes[0]);
        platform::precise_sleep(half_period);

        // 4. Второй фронт
        lpt_port_.write(step_bytes[1]);
        platform::precise_sleep(half_period);
    }


    std::uint8_t G540LptMotorDriver::readState() const {
        constexpr std::uint8_t invert_mask = 1 << 7;
        return lpt_port_.read(1) ^ invert_mask;
    }

    void G540LptMotorDriver::handleLimitEvents(const MotorLimitsState& current)
    {
        // 1. Фильтрация повторяющегося состояния
        if (current == last_limits_state_)
            return;

        // 2. Логирование изменения состояния лимитов
        logger_.info("Limit state changed: home={}, end={}",
                     current.home, current.end);

        // 3. Генерация доменного события LimitsChanged
        MotorDriverEvent::LimitsChanged ev;
        ev.limits = current;
        notifier_.notifyEvent(MotorDriverEvent(ev));

        // 4. Обновление последнего состояния
        last_limits_state_ = current;
    }
}
