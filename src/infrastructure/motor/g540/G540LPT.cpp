#include "G540LPT.h"
#include "g540logic.h"
#include <algorithm>
#include <atomic>
#include <thread>
#include <infrastructure/platform/sleep/sleep.h>

#include "infrastructure/platform/lpt/LptPort.h"


namespace infra::hardware {

    struct G540LPT::G540LptImpl {
        explicit G540LptImpl(unsigned short lpt_port)
            : lpt_port(lpt_port)
        { }

        platform::LptPort lpt_port;

        std::atomic<bool> emergency{false};
        std::atomic<bool> stopped{true};
        mutable std::thread worker;

        std::atomic<G540Direction> direction;
        std::atomic<std::chrono::steady_clock::duration> half_period{};
        struct {std::atomic<unsigned char> b1; std::atomic<unsigned char> b2; } step_bytes{};
        struct {std::atomic<unsigned char> begin; std::atomic<unsigned char> end; } limit_bytes{};
    };

    G540LPT::G540LPT(const G540Ports& ports, const G540LptConfig &config)
        : impl_(std::make_unique<G540LptImpl>(config.lpt_port))
        , ports_(ports)
        , config_(config)
        , logger_(ports.logger)
    {
        logger_.info("constructor called");
        impl_->limit_bytes.begin = 1 << config_.bit_begin_limit_switch;
        impl_->limit_bytes.end   = 1 << config_.bit_end_limit_switch;
        G540LPT::applyDirection(G540Direction::Neutral);
        G540LPT::applyFrequency(config_.min_freq_hz);
    }

    G540LPT::~G540LPT() {
        logger_.info("destructor called, stopping worker");
        G540LPT::stop();
    }

    void G540LPT::start() {
        if (!impl_->stopped) {
            logger_.warn("start() called but worker already running");
            return;
        }

        logger_.info("starting worker thread");

        if (impl_->worker.joinable()) {
            impl_->worker.join();
        }

        impl_->stopped.store(false, std::memory_order_release);
        impl_->emergency.store(false, std::memory_order_release);
        impl_->worker = std::thread(&G540LPT::run, this);
    }

    void G540LPT::stop() {
        if (impl_->stopped.exchange(true, std::memory_order_release)) {
            logger_.warn("stop() called but worker already stopped");
            return;
        }

        logger_.info("stopping worker thread");

        if (impl_->worker.joinable()) {
            impl_->worker.join();
        }

        applyDirection(G540Direction::Neutral);
        applyFrequency(config_.min_freq_hz);
    }

    void G540LPT::emergencyStop() {
        impl_->emergency.store(true, std::memory_order_release);
        impl_->stopped.store(true, std::memory_order_release);

        // Немедленно гасим импульсы
        // impl_->lpt_port.write(0, 0);

        logger_.error("!!! EMERGENCY STOP !!!");
    }

    bool G540LPT::stopped() const {
        return impl_->stopped.load(std::memory_order_acquire);
    }

    void G540LPT::applyFrequency(int hz) {
        if (hz > config_.max_freq_hz) hz = config_.max_freq_hz;
        if (hz < config_.min_freq_hz) hz = config_.min_freq_hz;
        impl_->half_period = g540logic::calculateHalfPeriod(hz);
        logger_.info("frequency set to {} Hz", hz);
    }

    void G540LPT::applyDirection(G540Direction direction) {
        constexpr int axis = 0; // Ось X
        constexpr int shift = 2 * axis; // 0, 2, 4, 6 для X,Y,Z,A;

        impl_->direction = direction;
        if (direction == G540Direction::Forward) {
            logger_.info("direction set to Forward");
            impl_->step_bytes.b1 = 0 << shift;
            impl_->step_bytes.b2 = 1 << shift;
        }
        else if (direction == G540Direction::Backward) {
            logger_.info("direction set to Backward");
            impl_->step_bytes.b1 = 2 << shift;
            impl_->step_bytes.b2 = 3 << shift;
        }
        else {
            logger_.info("direction set to Neutral");
            impl_->step_bytes.b1 = 0;
            impl_->step_bytes.b2 = 0;
        }
    }

    void G540LPT::applyFlapsState(G540FlapsState flaps_state) {
        switch (flaps_state) {
            case G540FlapsState::CloseBoth:
                logger_.info("flaps state set to CloseBoth");
                impl_->lpt_port.write(2, config_.byte_close_both_flaps);
                break;
            case G540FlapsState::OpenInput:
                logger_.info("flaps state set to OpenInput");
                impl_->lpt_port.write(2, config_.byte_open_input_flap);
                break;
            case G540FlapsState::OpenOutput:
                logger_.info("flaps state set to OpenOutput");
                impl_->lpt_port.write(2, config_.byte_open_output_flap);
                break;
            default:
                logger_.error("invalid G540StepperFlapsState argument");
        }
    }

    G540LimitsState G540LPT::getLimitsState() const {
        const unsigned char state = readState();
        const bool begin = state & impl_->limit_bytes.begin.load();
        const bool end   = state & impl_->limit_bytes.end.load();
        return  begin && end ? G540LimitsState::Both  :
                begin        ? G540LimitsState::Begin :
                end          ? G540LimitsState::End   :
                               G540LimitsState::None;
    }

    void G540LPT::run() {
        using namespace std::chrono;

        logger_.info("worker thread started");

        while (true)
        {
            if (impl_->emergency.load(std::memory_order_acquire)) break;
            if (impl_->stopped.load(std::memory_order_acquire)) break;

            const auto direction = impl_->direction.load();
            const auto half_period = impl_->half_period.load();
            const auto limits    = getLimitsState();
            const auto step1 = impl_->step_bytes.b1.load();
            const auto step2 = impl_->step_bytes.b2.load();

            if (!g540logic::canMove(direction, limits)) {
                platform::sleep(25ms);
                continue;
            }

            if (impl_->emergency.load(std::memory_order_acquire)) break;
            impl_->lpt_port.write(0, step1);
            platform::precise_sleep(half_period);

            if (impl_->emergency.load(std::memory_order_acquire)) break;
            impl_->lpt_port.write(0, step2);
            platform::precise_sleep(half_period);
        }

        logger_.info("worker thread stopped");
        impl_->stopped.store(true, std::memory_order_release);
    }

    unsigned char G540LPT::readState() const {
        return impl_->lpt_port.read(1) ^ (1 << 7);
    }
}
