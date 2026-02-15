#include "G540LptMotorDriver.h"
#include "../g540logic.h"
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <infrastructure/platform/sleep/sleep.h>

#include "infrastructure/platform/lpt/LptPort.h"


namespace infra::hardware {

    struct G540LptMotorDriver::G540LptImpl {

    // ---- ctor / dtor ----

    G540LptImpl()
    {
        worker = std::thread([this] { run(); });
    }

    ~G540LptImpl()
    {
        {
            std::lock_guard lock(mutex);
            shutdown = true;
            cv.notify_all();
        }

        if (worker.joinable())
            worker.join();
    }

    // ---- public API called from driver ----

    void start(domain::common::MotorDirection dir,
               std::chrono::nanoseconds halfPeriod)
    {
        std::lock_guard lock(mutex);

        direction = dir;
        this->halfPeriod = halfPeriod;
        running = true;
        emergency = false;

        cv.notify_all();
    }

    void stop()
    {
        std::lock_guard lock(mutex);
        running = false;
        cv.notify_all();
    }

    void emergencyStop()
    {
        std::lock_guard lock(mutex);
        emergency = true;
        running = false;
        cv.notify_all();
    }

    void setHalfPeriod(std::chrono::nanoseconds hp)
    {
        std::lock_guard lock(mutex);
        halfPeriod = hp;
    }

    domain::common::MotorLimitsState limits() const
    {
        std::lock_guard lock(mutex);
        return limitsState;
    }

    domain::common::MotorFault fault() const
    {
        std::lock_guard lock(mutex);
        return faultState;
    }

private:

    void run()
    {
        std::unique_lock lock(mutex);

        while (!shutdown)
        {
            cv.wait(lock, [&] { return running || shutdown; });

            if (shutdown)
                break;

            while (running && !emergency)
            {
                auto localHalfPeriod = halfPeriod;
                auto localDirection  = direction;

                lock.unlock();

                stepOnce(localDirection);

                std::this_thread::sleep_for(localHalfPeriod);

                checkLimits();
                checkFault();

                lock.lock();
            }
        }
    }

    void stepOnce(domain::common::MotorDirection dir)
    {
        // Здесь низкоуровневая запись в LPT
        // lpt_port.write(step_byte);
    }

    void checkLimits()
    {
        // читаем порт
        // обновляем limitsState под mutex
        std::lock_guard lock(mutex);
        // limitsState = ...
    }

    void checkFault()
    {
        std::lock_guard lock(mutex);
        // faultState = ...
    }

private:

    // ---- hardware ----
    platform::LptPort lpt_port;

    // ---- threading ----
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::thread worker;
    bool shutdown{false};

    // ---- state ----
    bool running{false};
    bool emergency{false};

    domain::common::MotorDirection direction{
        domain::common::MotorDirection::Forward
    };

    std::chrono::nanoseconds halfPeriod{0};

    domain::common::MotorLimitsState limitsState{};
    domain::common::MotorFault faultState{
        domain::common::MotorFault::None
    };
};


    G540LptMotorDriver::G540LptMotorDriver(const G540LptMotorDriverPorts& ports, const G540LptMotorDriverConfig &config)
        : impl_(std::make_unique<G540LptImpl>())
        , ports_(ports)
        , config_(config)
        , logger_(ports.logger)
    {
        logger_.info("constructor called");
        impl_->limit_bytes.begin = 1 << config_.bit_begin_limit_switch;
        impl_->limit_bytes.end   = 1 << config_.bit_end_limit_switch;
        G540LptMotorDriver::applyDirection(G540Direction::Neutral);
        G540LptMotorDriver::applyFrequency(config_.min_freq_hz);
    }

    G540LptMotorDriver::~G540LptMotorDriver() {
        logger_.info("destructor called, stopping worker");
        G540LptMotorDriver::stop();
    }

    void G540LptMotorDriver::start() {
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
        impl_->worker = std::thread(&G540LptMotorDriver::run, this);
    }

    void G540LptMotorDriver::stop() {
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

    void G540LptMotorDriver::emergencyStop() {
        impl_->emergency.store(true, std::memory_order_release);
        impl_->stopped.store(true, std::memory_order_release);

        // Немедленно гасим импульсы
        // impl_->lpt_port.write(0, 0);

        logger_.error("!!! EMERGENCY STOP !!!");
    }

    bool G540LptMotorDriver::stopped() const {
        return impl_->stopped.load(std::memory_order_acquire);
    }

    void G540LptMotorDriver::applyFrequency(int hz) {
        if (hz > config_.max_freq_hz) hz = config_.max_freq_hz;
        if (hz < config_.min_freq_hz) hz = config_.min_freq_hz;
        impl_->half_period = g540logic::calculateHalfPeriod(hz);
        logger_.info("frequency set to {} Hz", hz);
    }

    void G540LptMotorDriver::applyDirection(G540Direction direction) {
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

    void G540LptMotorDriver::applyFlapsState(G540FlapsState flaps_state) {
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

    G540LimitsState G540LptMotorDriver::getLimitsState() const {
        const unsigned char state = readState();
        const bool begin = state & impl_->limit_bytes.begin.load();
        const bool end   = state & impl_->limit_bytes.end.load();
        return  begin && end ? G540LimitsState::Both  :
                begin        ? G540LimitsState::Begin :
                end          ? G540LimitsState::End   :
                               G540LimitsState::None;
    }

    void G540LptMotorDriver::run() {
        using namespace std::chrono;

        logger_.info("worker thread started");

        logger_.info("opening LPT port");

        try {
            impl_->lpt_port.open(config_.lpt_port);
        }
        catch (std::exception &e) {
            logger_.error("failed to open COM port: {}", e.what());
            // notifier_.notifyOpenFailed({logger_.lastError()});
            return;
        }

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


        logger_.info("closing LPT port");
        impl_->lpt_port.close();

        logger_.info("worker thread stopped");
        impl_->stopped.store(true, std::memory_order_release);
    }

    unsigned char G540LptMotorDriver::readState() const {
        return impl_->lpt_port.read(1) ^ (1 << 7);
    }
}
