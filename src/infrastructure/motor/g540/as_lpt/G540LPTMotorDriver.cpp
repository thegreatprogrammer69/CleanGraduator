#include "G540LptMotorDriver.h"
#include "G540LptMotorDriver.h"
#include "domain/core/drivers/motor/MotorDriverEvent.h"
#include "infrastructure/platform/sleep/sleep.h"

namespace {
    using namespace domain::common;
    void genStepBytes(std::uint8_t (&out)[2], MotorDirection dir) const {
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

    G540LPTMotorDriver::G540LPTMotorDriver(const MotorDriverPorts ports, const motors::G540LptMotorDriverConfig &config)
        : logger_(ports.logger), config_(config)
        , thread_worker_([this] () { this->loopOnce(); })
        , state_(MotorDriverState::Uninitialized)
        , frequency_(MotorFrequency(0))
    {
    }

    G540LPTMotorDriver::~G540LPTMotorDriver() {
        stop();
    }

    bool G540LPTMotorDriver::initialize() {
        try {
            lpt_port_.open(config_.lpt_port);
        }
        catch (const std::exception& e) {
            // log
            return false;
        }
        state_ = MotorDriverState::Stopped;
        return true;
    }

    bool G540LPTMotorDriver::start() {
        if (state_ == MotorDriverState::Uninitialized) {
            if (!initialize()) {
                // logger.error("{}", ...)
                return false;
            }
        }
        if (state_ == MotorDriverState::Running) {
            // log warning
            return true;
        }

        if (state_ == MotorDriverState::Stopped) {
            // log info - то что мы Stopped -> Running
        }

        resetError();

        if (!lpt_port_.isOpen()) lpt_port_.open(config_.lpt_port);

        if (!thread_worker_.isRunning()) {
            thread_worker_.start();
        }
        else {
            thread_worker_.resume();
        }

        state_ = MotorDriverState::Running;

        return true;
    }

    void G540LPTMotorDriver::stop() {
        if (state_ == MotorDriverState::Stopped) {
            // log warning
        }
        thread_worker_.pause();
        state_ = MotorDriverState::Stopped;
    }

    void G540LPTMotorDriver::emergencyStop() {
        thread_worker_.pause();
        state_ = MotorDriverState::Stopped;
        lpt_port_.close();
    }

    MotorDriverState G540LPTMotorDriver::state() const {
        return state_;
    }


    void G540LPTMotorDriver::resetError() {
        error_.update([](MotorDriverError& err) { err.reset(); });
    }

    MotorDriverError G540LPTMotorDriver::error() const {
        return error_.load();
    }

    void G540LPTMotorDriver::enableWatchdog(std::chrono::milliseconds timeout) {
        software_watchdog_.start(timeout);
    }

    void G540LPTMotorDriver::disableWatchdog() {
        software_watchdog_.stop();
    }

    void G540LPTMotorDriver::setFrequency(const MotorFrequency frequency) {
        frequency_.store(frequency);
    }

    MotorFrequency G540LPTMotorDriver::frequency() const {
        return frequency_.load();
    }

    MotorFrequencyLimits G540LPTMotorDriver::frequencyLimits() const {
        MotorFrequencyLimits frequency_limits;
        frequency_limits.maxHz = config_.max_freq_hz;
        frequency_limits.minHz = config_.min_freq_hz;
        return frequency_limits;
    }

    void G540LPTMotorDriver::setDirection(const MotorDirection dir) {
        direction_ = dir;
    }

    MotorDirection G540LPTMotorDriver::direction() const {
        return direction_;
    }

    void G540LPTMotorDriver::setFlapsState(MotorFlapsState state) {
        flaps_state_ = state;
    }

    MotorFlapsState G540LPTMotorDriver::flapsState() const {
        return flaps_state_;
    }

    MotorLimitsState G540LPTMotorDriver::limits() const {
        const auto state = readState();
        MotorLimitsState limits_state;
        limits_state.home = (state & config_.bit_begin_limit_switch) != 0;
        limits_state.end  = (state & config_.bit_end_limit_switch) != 0;
        return limits_state;
    }

    void G540LPTMotorDriver::addObserver(domain::ports::IMotorDriverObserver &o) {
        notifier_.addObserver(o);
    }

    void G540LPTMotorDriver::removeObserver(domain::ports::IMotorDriverObserver &o) {
        notifier_.removeObserver(o);
    }

    void G540LPTMotorDriver::loopOnce() {
        if (!pollSafety()) {
            stop();
            return;
        }
        stepOnce();
    }

    bool G540LPTMotorDriver::pollSafety() {
        if (software_watchdog_.expired()) {
            // logger_.error()

            MotorDriverError new_err;
            new_err.message = "Watchdog timeout";
            error_.store(new_err);

            MotorDriverEvent::Fault ev;
            ev.error = new_err;
            notifier_.notify(MotorDriverEvent(ev));

            return false;
        }

        const auto limits = this->limits();

        if (direction_ == MotorDirection::Forward && limits.end) {
            // logger_.info()

            MotorDriverEvent::LimitsChanged ev;
            ev.limits = limits;
            notifier_.notify(MotorDriverEvent(ev));

            return false;
        }

        if (direction_ == MotorDirection::Backward && limits.home) {
            // logger_.info()

            MotorDriverEvent::LimitsChanged ev;
            ev.limits = limits;
            notifier_.notify(MotorDriverEvent(ev));

            return false;
        }

        return true;
    }

    void G540LPTMotorDriver::stepOnce() {
        std::uint8_t step_bytes[2];
        genStepBytes(step_bytes, direction_);

        const auto half_period = frequency_.load().halfPeriod();

        lpt_port_.write(step_bytes[0]);
        platform::precise_sleep(half_period);

        lpt_port_.write(step_bytes[1]);
        platform::precise_sleep(half_period);
    }



    std::uint8_t G540LPTMotorDriver::readState() const {
        constexpr std::uint8_t invert_mask = 1 << 7;
        return lpt_port_.read(1) ^ invert_mask;
    }
}
