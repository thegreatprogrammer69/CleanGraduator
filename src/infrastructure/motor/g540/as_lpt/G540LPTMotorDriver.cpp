#include "G540LptMotorDriver.h"

namespace infra::motor {
    using namespace domain::common;

    G540LPTMotorDriver::G540LPTMotorDriver(MotorDriverPorts ports, motors::G540LptMotorDriverConfig config)
        : logger_(ports.logger), config_(config)
        , state_(MotorDriverState::Uninitialized)
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
        if (state_ == MotorDriverState::Fault) {
            if (!resetError()) {
                // log error
                return false;
            }
            // log info - то что мы Fault -> Running
        }
        if (state_ == MotorDriverState::Stopped) {
            // log info - то что мы Stopped -> Running
        }

        if (!thread_worker_.isRunning()) {
            thread_worker_.start();
        }
        else {
            thread_worker_.resume();
        }

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


    bool G540LPTMotorDriver::resetError() {
        error_.update([](MotorDriverError& err) {err.reset(); });
        return true;
    }

    MotorDriverError G540LPTMotorDriver::error() const {
        return error_.load();
    }

    void G540LPTMotorDriver::enableWatchdog(std::chrono::milliseconds timeout) {
    }

    void G540LPTMotorDriver::disableWatchdog() {
    }

    void G540LPTMotorDriver::setFrequency(MotorFrequency frequency) {
    }

    MotorFrequency G540LPTMotorDriver::frequency() const {
        reurn frequency_.load();
    }

    MotorFrequencyLimits G540LPTMotorDriver::frequencyLimits() const {
    }

    void G540LPTMotorDriver::setDirection(MotorDirection dir) {
    }

    MotorDirection G540LPTMotorDriver::direction() const {
    }

    void G540LPTMotorDriver::setFlapsState(MotorFlapsState state) {
    }

    MotorFlapsState G540LPTMotorDriver::flapsState() const {
    }

    MotorLimitsState G540LPTMotorDriver::limits() const {
    }

    void G540LPTMotorDriver::addObserver(domain::ports::IMotorDriverObserver &o) {
        notifier_.addObserver(o);
    }

    void G540LPTMotorDriver::removeObserver(domain::ports::IMotorDriverObserver &o) {
        notifier_.removeObserver(o);
    }
}