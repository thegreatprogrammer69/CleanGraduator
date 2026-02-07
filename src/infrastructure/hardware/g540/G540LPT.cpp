#include "G540LPT.h"
#include "g540logic.h"
#include <algorithm>
#include <infrastructure/platform/sleep/sleep.h>


namespace infra::hardware {
    G540LPT::G540LPT(const G540Ports& ports, const G540LptConfig &config)
        : ports_(ports)
        , config_(config)
        , lpt_port_(config.lpt_port)
    {
        ports_.logger.info("constructor called");
        limit_bytes_.begin = 1 << config_.bit_begin_limit_switch;
        limit_bytes_.end   = 1 << config_.bit_end_limit_switch;
        G540LPT::applyDirection(G540Direction::Neutral);
        G540LPT::applyFrequency(config_.min_freq_hz);
    }

    G540LPT::~G540LPT() {
        ports_.logger.info("destructor called, stopping worker");
        G540LPT::stop();
    }

    void G540LPT::start() {
        if (!stopped_) {
            ports_.logger.warn("start() called but worker already running");
            return;
        }

        ports_.logger.info("starting worker thread");

        if (worker_.joinable()) {
            worker_.join();
        }

        stopped_.store(false, std::memory_order_release);
        worker_ = std::thread(&G540LPT::run, this);
    }

    void G540LPT::stop() {
        if (stopped_.exchange(true, std::memory_order_relaxed)) {
            ports_.logger.warn("stop() called but worker already stopped");
            return;
        }

        ports_.logger.info("stopping worker thread");

        if (worker_.joinable()) {
            worker_.join();
        }

        applyDirection(G540Direction::Neutral);
        applyFrequency(config_.min_freq_hz);
    }

    void G540LPT::emergencyStop() {
        stopped_.store(true, std::memory_order_relaxed);
        // lpt_port_.write(0, 0);
        ports_.logger.error("EMERGENCY STOP triggered");
    }

    bool G540LPT::stopped() const {
        return stopped_.load(std::memory_order_acquire);
    }

    void G540LPT::applyFrequency(int hz) {
        if (hz > config_.max_freq_hz) hz = config_.max_freq_hz;
        if (hz < config_.min_freq_hz) hz = config_.min_freq_hz;
        half_period_ = g540logic::calculateHalfPeriod(hz);
        ports_.logger.info("frequency set to " + std::to_string(hz) + " Hz");
    }

    void G540LPT::applyDirection(G540Direction direction) {
        constexpr int axis = 0; // Ось X
        constexpr int shift = 2 * axis; // 0, 2, 4, 6 для X,Y,Z,A;

        direction_ = direction;
        if (direction == G540Direction::Forward) {
            ports_.logger.info("direction set to Forward");
            step_bytes_.b1 = 0 << shift;
            step_bytes_.b2 = 1 << shift;
        }
        else if (direction == G540Direction::Backward) {
            ports_.logger.info("direction set to Backward");
            step_bytes_.b1 = 2 << shift;
            step_bytes_.b2 = 3 << shift;
        }
        else {
            ports_.logger.info("direction set to Neutral");
            step_bytes_.b1 = 0;
            step_bytes_.b2 = 0;
        }
    }

    void G540LPT::applyFlapsState(G540FlapsState flaps_state) {
        switch (flaps_state) {
            case G540FlapsState::CloseBoth:
                ports_.logger.info("flaps state set to CloseBoth");
                lpt_port_.write(2, config_.byte_close_both_flaps);
                break;
            case G540FlapsState::OpenInput:
                ports_.logger.info("flaps state set to OpenInput");
                lpt_port_.write(2, config_.byte_open_input_flap);
                break;
            case G540FlapsState::OpenOutput:
                ports_.logger.info("flaps state set to OpenOutput");
                lpt_port_.write(2, config_.byte_open_output_flap);
                break;
            default:
                ports_.logger.error("invalid G540StepperFlapsState argument");
        }
    }

    G540LimitsState G540LPT::getLimitsState() const {
        const unsigned char state = readState();
        const bool begin = state & limit_bytes_.begin.load();
        const bool end   = state & limit_bytes_.end.load();
        return  begin && end ? G540LimitsState::Both  :
                begin        ? G540LimitsState::Begin :
                end          ? G540LimitsState::End   :
                               G540LimitsState::None;
    }

    void G540LPT::run() {
        using namespace std::chrono;

        ports_.logger.info("worker thread started");

        while (!stopped_.load(std::memory_order_acquire))
        {
            const auto direction = direction_.load();
            const auto half_period = half_period_.load();
            const auto limits    = getLimitsState();
            const auto step1 = step_bytes_.b1.load();
            const auto step2 = step_bytes_.b2.load();

            if (!g540logic::canMove(direction, limits)) {
                platform::sleep(100ms);
                continue;
            }

            lpt_port_.write(0, step1);
            platform::precise_sleep(half_period);

            lpt_port_.write(0, step2);
            platform::precise_sleep(half_period);
        }

        ports_.logger.info("worker thread stopped");
        stopped_.store(true, std::memory_order_release);
    }

    unsigned char G540LPT::readState() const {
        return lpt_port_.read(1) ^ (1 << 7);
    }
}
