#include "G540LptMotorDriver.h"

#include "infrastructure/platform/lpt/LptPort.h"
#include "infrastructure/platform/sleep/sleep.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

#include "domain/ports/motor/IMotorDriverObserver.h"
#include "domain/ports/motor/IValveDriverObserver.h"

using namespace infra::motors;

namespace {

inline int clampHz(int hz, int minHz, int maxHz) {
    return std::max(minHz, std::min(maxHz, hz));
}

inline unsigned char invertBusyBit(unsigned char s) {
    // раннее readState(): read(1) ^ (1<<7)
    return static_cast<unsigned char>(s ^ (1u << 7));
}

inline domain::common::MotorLimitsState toLimitsState(unsigned char state,
                                                      unsigned char beginMask,
                                                      unsigned char endMask)
{
    domain::common::MotorLimitsState out{};
    out.home = (state & beginMask) != 0; // backward limit (home)
    out.end  = (state & endMask)   != 0; // forward  limit (end)
    return out;
}

inline bool canMove(domain::common::MotorDirection direction,
                    const domain::common::MotorLimitsState& limits)
{
    using domain::common::MotorDirection;

    switch (direction)
    {
        case MotorDirection::Forward:
            return !limits.end;

        case MotorDirection::Backward:
            return !limits.home;
    }

    return false;
}

} // namespace

// ==========================================================
// Impl
// ==========================================================
//
// Поток всегда живёт.
// В горячем пути (runLoop) не берём мьютексы.
// Ошибка (fault.error) хранится отдельно и читается только по запросу,
// чтобы не таскать std::string через атомики.
//

struct G540LptMotorDriver::G540LptImpl {
    explicit G540LptImpl(G540LptMotorDriver& owner,
                         domain::ports::ILogger& logger,
                         const G540LptMotorDriverConfig& cfg)
        : owner_(owner)
        , log_(logger)
        , cfg_(cfg)
        , begin_mask_(static_cast<unsigned char>(1u << cfg_.bit_begin_limit_switch))
        , end_mask_(static_cast<unsigned char>(1u << cfg_.bit_end_limit_switch))
        , half_period_ns_(halfPeriodNsCount(clampHz(cfg_.min_freq_hz, cfg_.min_freq_hz, cfg_.max_freq_hz)))
        , direction_(static_cast<DirU>(domain::common::MotorDirection::Forward))
        , shared_state_(packInitialState())
    {
        worker_ = std::thread([this]{ threadMain(); });
    }

    ~G540LptImpl() { shutdownAndJoin(); }

    // =========================
    // API (external thread)
    // =========================
    void start() {
        requested_emergency_.store(false, std::memory_order_relaxed);
        requested_running_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

    void stop() {
        requested_running_.store(false, std::memory_order_relaxed);
        // stop может не разбудить wait, но если не бежим — и не надо.
        cv_.notify_all();
    }

    bool isRunning() const {
        return requested_running_.load(std::memory_order_relaxed);
    }

    void emergencyStop() {
        requested_running_.store(false, std::memory_order_relaxed);
        requested_emergency_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

    void resetFault() {
        const auto prev = exchangeFaultType(FaultType::None);

        {
            std::lock_guard lk(fault_mtx_);
            fault_error_.clear();
        }

        if (prev != FaultType::None) {
            domain::common::MotorFault f{};
            f.type = FaultType::None;
            notifyFault(f);
        }
    }

    void setDirection(domain::common::MotorDirection d) {
        const auto old = direction_.exchange(
            static_cast<DirU>(d),
            std::memory_order_relaxed);

        if (old != static_cast<DirU>(d)) {
            notifyDirectionChanged(d);
        }
    }

    void setHz(int hz) {
        hz = clampHz(hz, cfg_.min_freq_hz, cfg_.max_freq_hz);
        half_period_ns_.store(halfPeriodNsCount(hz), std::memory_order_relaxed);
    }

    void setFlaps(FlapsState s) {
        setFlapsBits(s);
        flaps_dirty_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

    domain::common::MotorLimitsState limits() const {
        const auto st = shared_state_.load(std::memory_order_relaxed);
        domain::common::MotorLimitsState out{};
        out.home = (st & LIMIT_HOME_BIT) != 0;
        out.end  = (st & LIMIT_END_BIT)  != 0;
        return out;
    }

    domain::common::MotorFault fault() const {
        domain::common::MotorFault out{};
        out.type = faultType();

        if (out.type == FaultType::DriverError) {
            std::lock_guard lk(fault_mtx_);
            out.error = fault_error_;
        } else {
            out.error.clear();
        }

        return out;
    }

    void addObserver(domain::ports::IMotorDriverObserver& o) {
        std::lock_guard lk(obs_mtx_);
        observers_.push_back(&o);
    }

    void removeObserver(domain::ports::IMotorDriverObserver& o) {
        std::lock_guard lk(obs_mtx_);
        observers_.erase(std::remove(observers_.begin(), observers_.end(), &o), observers_.end());
    }

    void addObserver(domain::ports::IValveDriverObserver& o) {
        std::lock_guard lk(valve_obs_mtx_);
        valve_observers_.push_back(&o);
    }

    void removeObserver(domain::ports::IValveDriverObserver& o) {
        std::lock_guard lk(valve_obs_mtx_);
        valve_observers_.erase(std::remove(valve_observers_.begin(), valve_observers_.end(), &o), valve_observers_.end());
    }

private:
    using Dir      = domain::common::MotorDirection;
    using DirU     = std::underlying_type_t<Dir>;
    using FaultType = domain::common::MotorFaultType;
    using FaultU   = std::underlying_type_t<FaultType>;

    // =========================
    // Packed shared state in shared_state_ (single allowed atomic)
    // bits:
    // 0: home limit
    // 1: end limit
    // 8..23: fault type (16 bits)
    // 24..25: flaps (2 bits)
    // =========================
    static constexpr std::uint32_t LIMIT_HOME_BIT = 1u << 0;
    static constexpr std::uint32_t LIMIT_END_BIT  = 1u << 1;

    static constexpr std::uint32_t FAULT_SHIFT = 8;
    static constexpr std::uint32_t FAULT_MASK  = 0xFFFFu << FAULT_SHIFT;

    static constexpr std::uint32_t FLAPS_SHIFT = 24;
    static constexpr std::uint32_t FLAPS_MASK  = 0x03u   << FLAPS_SHIFT;

    static std::int64_t halfPeriodNsCount(int hz) {
        if (hz <= 0) hz = 1;
        const std::int64_t denom = static_cast<std::int64_t>(hz) * 2;
        std::int64_t ns = 1'000'000'000LL / denom;
        if (ns <= 0) ns = 1;
        return ns;
    }

    static constexpr std::uint32_t packFaultBits(FaultType t) {
        return (static_cast<std::uint32_t>(static_cast<FaultU>(t)) & 0xFFFFu) << FAULT_SHIFT;
    }

    static constexpr std::uint32_t packFlapsBits(FlapsState s) {
        return (static_cast<std::uint32_t>(s) & 0x03u) << FLAPS_SHIFT;
    }

    static constexpr std::uint32_t packInitialState() {
        return packFaultBits(FaultType::None) | packFlapsBits(FlapsState::CloseBoth);
    }

    FaultType faultType() const {
        const auto st  = shared_state_.load(std::memory_order_relaxed);
        const auto raw = static_cast<FaultU>((st & FAULT_MASK) >> FAULT_SHIFT);
        return static_cast<FaultType>(raw);
    }

    FaultType exchangeFaultType(FaultType t) {
        const std::uint32_t newBits = packFaultBits(t);

        std::uint32_t old = shared_state_.load(std::memory_order_relaxed);
        for (;;) {
            const auto prevRaw = static_cast<FaultU>((old & FAULT_MASK) >> FAULT_SHIFT);
            const auto prev    = static_cast<FaultType>(prevRaw);

            const std::uint32_t neu = (old & ~FAULT_MASK) | newBits;

            if (shared_state_.compare_exchange_weak(old, neu,
                                                   std::memory_order_relaxed,
                                                   std::memory_order_relaxed))
                return prev;
        }
    }

    void setFlapsBits(FlapsState s) {
        const std::uint32_t newFlaps = packFlapsBits(s);
        std::uint32_t old = shared_state_.load(std::memory_order_relaxed);
        for (;;) {
            const std::uint32_t neu = (old & ~FLAPS_MASK) | newFlaps;
            if (shared_state_.compare_exchange_weak(old, neu,
                                                   std::memory_order_relaxed,
                                                   std::memory_order_relaxed))
                return;
        }
    }

    FlapsState currentFlaps() const {
        const auto st = shared_state_.load(std::memory_order_relaxed);
        return static_cast<FlapsState>((st & FLAPS_MASK) >> FLAPS_SHIFT);
    }

    // publish limits only when changed (avoid CAS every step)
    void publishLimitsIfChanged(bool home, bool end) {
        std::uint8_t bits = 0;
        if (home) bits |= 0x01u;
        if (end)  bits |= 0x02u;

        if (bits == last_limits_bits_) return; // fast path
        last_limits_bits_ = bits;

        const std::uint32_t newLimitBits =
            (home ? LIMIT_HOME_BIT : 0u) | (end ? LIMIT_END_BIT : 0u);

        std::uint32_t old = shared_state_.load(std::memory_order_relaxed);
        for (;;) {
            const std::uint32_t neu =
                (old & ~(LIMIT_HOME_BIT | LIMIT_END_BIT)) | newLimitBits;

            if (shared_state_.compare_exchange_weak(
                    old, neu,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed))
            {
                // 👉 ДОБАВИТЬ ЭТО:
                domain::common::MotorLimitsState s{};
                s.home = home;
                s.end  = end;
                notifyLimitsChanged(s);

                return;
            }
        }
    }


    // =========================
    // Thread control
    // =========================
    void shutdownAndJoin() {
        shutdown_.store(true, std::memory_order_relaxed);
        requested_running_.store(false, std::memory_order_relaxed);
        cv_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

    // =========================
    // Observers (mutex OK, rare)
    // =========================
    template<class Fn>
    void notifyAll(Fn&& fn) {
        std::vector<domain::ports::IMotorDriverObserver*> copy;
        {
            std::lock_guard lk(obs_mtx_);
            copy = observers_;
        }
        for (auto* o : copy) fn(*o);
    }
    template<class Fn>
    void notifyAllValve(Fn&& fn) {
        std::vector<domain::ports::IValveDriverObserver*> copy;
        {
            std::lock_guard lk(valve_obs_mtx_);
            copy = valve_observers_;
        }
        for (auto* o : copy) fn(*o);
    }

    void notifyInputFlapOpened() { notifyAllValve([](auto& o){ o.onInputFlapOpened(); }); }
    void notifyOutputFlapOpened() { notifyAllValve([](auto& o){ o.onOutputFlapOpened(); }); }
    void notifyFlapsClosed() { notifyAllValve([](auto& o){ o.onFlapsClosed(); }); }
    void notifyStarted()  { notifyAll([](auto& o){ o.onStarted(); }); }
    void notifyStopped()  { notifyAll([](auto& o){ o.onStopped(); }); }
    void notifyLimitsChanged(domain::common::MotorLimitsState s)  { notifyAll([s](auto& o){ o.onLimitsStateChanged(s); }); }
    void notifyDirectionChanged(domain::common::MotorDirection d)  { notifyAll([d](auto& o){ o.onDirectionChanged(d); }); }
    void notifyFault(const domain::common::MotorFault& f) {
        notifyAll([&](auto& o){ o.onFault(f); });
    }

    // =========================
    // LPT low-level (worker-only)
    // =========================
    unsigned char readState() {
        return invertBusyBit(lpt_.read(1));
    }

    void writeNeutral() {
        lpt_.write(0, 0);
    }

    void applyFlaps(FlapsState s) {
        switch (s) {
            case FlapsState::CloseBoth:
                lpt_.write(2, cfg_.byte_close_both_flaps);
                notifyFlapsClosed();
                break;
            case FlapsState::OpenInput:
                lpt_.write(2, cfg_.byte_open_input_flap);
                notifyInputFlapOpened();
                break;
            case FlapsState::OpenOutput:
                lpt_.write(2, cfg_.byte_open_output_flap);
                notifyOutputFlapOpened();
                break;
        }
    }

    void stepOnce(Dir dir, std::chrono::nanoseconds halfPeriod) {
        constexpr int axis  = 0;
        constexpr int shift = 2 * axis;

        const int d = static_cast<int>(dir);
        const unsigned char b1 = static_cast<unsigned char>(((d * 2 + 0) << shift) & 0xFF);
        const unsigned char b2 = static_cast<unsigned char>(((d * 2 + 1) << shift) & 0xFF);

        lpt_.write(0, b1);
        platform::precise_sleep(halfPeriod);

        lpt_.write(0, b2);
        platform::precise_sleep(halfPeriod);
    }

    // =========================
    // Worker main
    // =========================
    void threadMain() {
        log_.info("G540Lpt worker started");

        if (!openPort()) {
            log_.error("G540Lpt worker exits: cannot open LPT");
            return;
        }

        // применим флапсы сразу после открытия порта
        applyFlaps(currentFlaps());

        for (;;) {
            // ждём события управления/остановки/клапанов
            {
                std::unique_lock lk(cv_mtx_);
                cv_.wait(lk, [&]{
                    return shutdown_.load(std::memory_order_relaxed)
                        || requested_running_.load(std::memory_order_relaxed)
                        || requested_emergency_.load(std::memory_order_relaxed)
                        || flaps_dirty_.load(std::memory_order_relaxed);
                });
            }

            if (shutdown_.load(std::memory_order_relaxed))
                break;

            // emergency приоритетнее всего
            if (requested_emergency_.load(std::memory_order_relaxed)) {
                handleEmergency();
                continue;
            }

            // клапаны должны работать даже без start()
            if (flaps_dirty_.exchange(false, std::memory_order_relaxed)) {
                applyFlaps(currentFlaps());
            }

            // движение включаем только если asked
            if (requested_running_.load(std::memory_order_relaxed)) {
                notifyStarted();
                runLoop(); // HOT PATH: no mutex inside
            }
        }

        writeNeutral();
        closePort();

        log_.info("G540Lpt worker stopped");
    }

    bool openPort() {
        if (lpt_.isOpen()) return true;
        try {
            lpt_.open(cfg_.lpt_port);
            lpt_opened_ = true;
            log_.info("LPT port opened");
            return true;
        } catch (const std::exception& e) {
            // выставляем fault type + error (error читается только при type == DriverError)
            exchangeFaultType(FaultType::DriverError);
            {
                std::lock_guard lk(fault_mtx_);
                fault_error_ = e.what();
            }

            domain::common::MotorFault f{};
            f.type = FaultType::DriverError;
            f.error = e.what();

            log_.error("failed to open LPT port: {}", e.what());
            notifyFault(f);

            return false;
        }
    }

    void closePort() {
        if (!lpt_opened_) return;
        lpt_.close();
        log_.info("LPT port closed");
    }

    void handleEmergency() {
        requested_emergency_.store(false, std::memory_order_relaxed);
        requested_running_.store(false, std::memory_order_relaxed);

        writeNeutral();

        const auto st  = readState();
        const auto lim = toLimitsState(st, begin_mask_, end_mask_);
        publishLimitsIfChanged(lim.home, lim.end);

        const auto prev = exchangeFaultType(FaultType::EmergencyStop);

        log_.error("!!! EMERGENCY STOP !!!");

        if (prev != FaultType::EmergencyStop) {
            domain::common::MotorFault f{};
            f.type = FaultType::EmergencyStop;
            notifyFault(f);
        }

        notifyStopped();
    }

    // =========================
    // HOT PATH: NO MUTEX HERE
    // =========================
    void runLoop() {
        for (;;) {
            if (shutdown_.load(std::memory_order_relaxed)) {
                writeNeutral();
                return;
            }

            // клапаны на лету
            if (flaps_dirty_.exchange(false, std::memory_order_relaxed)) {
                applyFlaps(currentFlaps());
            }

            if (requested_emergency_.load(std::memory_order_relaxed)) {
                handleEmergency();
                return;
            }

            if (!requested_running_.load(std::memory_order_relaxed)) {
                writeNeutral();
                notifyStopped();
                return;
            }

            const auto dir = static_cast<Dir>(direction_.load(std::memory_order_relaxed));
            const auto hp  = std::chrono::nanoseconds(half_period_ns_.load(std::memory_order_relaxed));

            const auto st  = readState();
            const auto lim = toLimitsState(st, begin_mask_, end_mask_);

            publishLimitsIfChanged(lim.home, lim.end);

            if (!canMove(dir, lim)) {
                platform::sleep(std::chrono::milliseconds(25));
                continue;
            }

            stepOnce(dir, hp);
        }
    }

private:
    G540LptMotorDriver& owner_;
    fmt::Logger log_;

    // hardware (worker-only)
    platform::LptPort lpt_;
    bool lpt_opened_{false};

    // immutable config
    const G540LptMotorDriverConfig cfg_;
    const unsigned char begin_mask_;
    const unsigned char end_mask_;

    // worker-local cache to avoid CAS each iteration
    std::uint8_t last_limits_bits_{0};

    // CV mutex (только для wait)
    mutable std::mutex cv_mtx_;
    std::condition_variable cv_;

    // observers mutex (редко)
    mutable std::mutex obs_mtx_;
    std::vector<domain::ports::IMotorDriverObserver*> observers_;
    mutable std::mutex valve_obs_mtx_;
    std::vector<domain::ports::IValveDriverObserver*> valve_observers_;

    // fault error (string) mutex (редко)
    mutable std::mutex fault_mtx_;
    std::string fault_error_;

    std::thread worker_;

    // ====== allowed atomics only ======
    std::atomic<bool> shutdown_{false};
    std::atomic<bool> requested_running_{false};
    std::atomic<bool> requested_emergency_{false};

    std::atomic<std::int64_t> half_period_ns_;
    std::atomic<DirU> direction_;

    std::atomic<bool> flaps_dirty_{false};
    std::atomic<std::uint32_t> shared_state_;
};

// ==========================================================
// Driver
// ==========================================================

G540LptMotorDriver::G540LptMotorDriver(const MotorDriverPorts& ports,
                                       const G540LptMotorDriverConfig& config)
    : logger_(ports.logger)
    , impl_(std::make_unique<G540LptImpl>(*this, ports.logger, config))
    , ports_(ports)
    , config_(config)
    , current_hz_(clampHz(config.min_freq_hz, config.min_freq_hz, config.max_freq_hz))
    , current_dir_(domain::common::MotorDirection::Forward)
{
    logger_.info("G540LptMotorDriver constructed");

    impl_->setDirection(current_dir_);
    impl_->setHz(config_.min_freq_hz);
}

G540LptMotorDriver::~G540LptMotorDriver() {
    logger_.info("G540LptMotorDriver destructing");
    // impl_ корректно остановит поток в своём деструкторе
}

void G540LptMotorDriver::start() {
    logger_.info("start()");
    impl_->start();
}

void G540LptMotorDriver::stop() {
    logger_.info("stop()");
    impl_->stop();
}

bool G540LptMotorDriver::isRunning() const {
    return impl_->isRunning();
}

void G540LptMotorDriver::emergencyStop() {
    logger_.error("emergencyStop()");
    impl_->emergencyStop();
}

void G540LptMotorDriver::setFrequency(int hz) {
    hz = clampHz(hz, config_.min_freq_hz, config_.max_freq_hz);
    current_hz_ = hz;
    logger_.info("setFrequency({})", hz);
    impl_->setHz(hz);
}

int G540LptMotorDriver::frequency() const {
    return current_hz_;
}

domain::common::FrequencyLimits G540LptMotorDriver::frequencyLimits() const {
    return { config_.min_freq_hz, config_.max_freq_hz };
}

void G540LptMotorDriver::setDirection(domain::common::MotorDirection dir) {
    current_dir_ = dir;
    logger_.info("setDirection({})", dir);
    impl_->setDirection(dir);
}

domain::common::MotorDirection G540LptMotorDriver::direction() const {
    return current_dir_;
}

domain::common::MotorLimitsState G540LptMotorDriver::limits() const {
    return impl_->limits();
}

domain::common::MotorFault G540LptMotorDriver::fault() const {
    return impl_->fault();
}

void G540LptMotorDriver::resetFault() {
    logger_.info("resetFault()");
    impl_->resetFault();
}

void G540LptMotorDriver::enableWatchdog(std::chrono::milliseconds) {
    // no-op
}

void G540LptMotorDriver::disableWatchdog() {
    // no-op
}

void G540LptMotorDriver::feedWatchdog() {
    // no-op
}

void G540LptMotorDriver::addObserver(domain::ports::IMotorDriverObserver &o) {
    impl_->addObserver(o);
}

void G540LptMotorDriver::removeObserver(domain::ports::IMotorDriverObserver &o) {
    impl_->removeObserver(o);
}

void G540LptMotorDriver::addObserver(domain::ports::IValveDriverObserver &o) {
    impl_->addObserver(o);
}

void G540LptMotorDriver::removeObserver(domain::ports::IValveDriverObserver &o) {
    impl_->removeObserver(o);
}

void G540LptMotorDriver::setFlapsState(FlapsState s) {
    impl_->setFlaps(s);
}

void G540LptMotorDriver::openInputFlap() {
    logger_.info("openInputFlap()");
    setFlapsState(FlapsState::OpenInput);
}

void G540LptMotorDriver::openOutputFlap() {
    logger_.info("openOutputFlap()");
    setFlapsState(FlapsState::OpenOutput);
}

void G540LptMotorDriver::closeFlaps() {
    logger_.info("closeFlaps()");
    setFlapsState(FlapsState::CloseBoth);
}
