#include "G540LptMotorDriver.h"

#include "infrastructure/platform/lpt/LptPort.h"
#include "infrastructure/platform/sleep/sleep.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <cstdint>
#include <type_traits>


#include "domain/ports/motor/IMotorDriverObserver.h"

using namespace infra::motors;

namespace {

inline int clampHz(int hz, int minHz, int maxHz) {
    return std::max(minHz, std::min(maxHz, hz));
}

inline unsigned char invertBusyBit(unsigned char s) {
    // раннее readState(): read(1) ^ (1<<7)
    return static_cast<unsigned char>(s ^ (1u << 7));
}

// кодирование лимитов в MotorLimitsState как у тебя (begin/home, end/end)
inline domain::common::MotorLimitsState toLimitsState(unsigned char state,
                                                      unsigned char beginMask,
                                                      unsigned char endMask)
{
    domain::common::MotorLimitsState out{};
    out.home = (state & beginMask) != 0; // backward limit (home)
    out.end  = (state & endMask)   != 0; // forward  limit (end)
    return out;
}

// Можно ли двигаться с учётом направления и лимитов
inline bool canMove(domain::common::MotorDirection direction,
                        const domain::common::MotorLimitsState& limits)
{
    using domain::common::MotorDirection;

    switch (direction)
    {
        case MotorDirection::Forward:
            // Движение вперёд запрещено если активен конечный (end) лимит
            return !limits.end;

        case MotorDirection::Backward:
            // Движение назад запрещено если активен начальный (home) лимит
            return !limits.home;
    }

    return false;
}


// Подсчёт half-period
inline std::chrono::steady_clock::duration calculateHalfPeriod(int hz) {
    using namespace std::chrono;
    if (hz <= 0) hz = 1;
    return duration_cast<steady_clock::duration>(seconds(1) / (hz * 2));
}

} // namespace

// ==========================================================
// Impl
// ==========================================================

// ==========================================================
// Impl (no configure(), runLoop() without mutex)
// Allowed atomics only:
// requested_running_, requested_emergency_, shutdown_,
// half_period_ns_, direction_, flaps_dirty_, flaps_state_
// ==========================================================

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
        , flaps_state_(packInitialState())
    {
        // Вариант A: поток всегда жив
        worker_ = std::thread([this]{ threadMain(); });
    }

    ~G540LptImpl() { shutdownAndJoin(); }

    // =========================
    // API (external thread)
    // =========================
    void start() {
        // start теперь ТОЛЬКО включает движение
        requested_emergency_.store(false, std::memory_order_relaxed);
        requested_running_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

    void stop() {
        requested_running_.store(false, std::memory_order_relaxed);
        cv_.notify_all();
    }

    void emergencyStop() {
        requested_running_.store(false, std::memory_order_relaxed);
        requested_emergency_.store(true, std::memory_order_relaxed);
        cv_.notify_all();
    }

    void setDirection(domain::common::MotorDirection d) {
        direction_.store(static_cast<DirU>(d), std::memory_order_relaxed);
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
        const auto st = flaps_state_.load(std::memory_order_relaxed);
        domain::common::MotorLimitsState out{};
        out.home = (st & LIMIT_HOME_BIT) != 0;
        out.end  = (st & LIMIT_END_BIT)  != 0;
        return out;
    }

    domain::common::MotorFault fault() const {
        const auto st = flaps_state_.load(std::memory_order_relaxed);
        const auto raw = static_cast<FaultU>((st & FAULT_MASK) >> FAULT_SHIFT);
        return static_cast<domain::common::MotorFault>(raw);
    }

    void addObserver(domain::ports::IMotorDriverObserver& o) {
        std::lock_guard lk(mtx_);
        observers_.push_back(&o);
    }

    void removeObserver(domain::ports::IMotorDriverObserver& o) {
        std::lock_guard lk(mtx_);
        observers_.erase(std::remove(observers_.begin(), observers_.end(), &o), observers_.end());
    }

private:
    using Dir    = domain::common::MotorDirection;
    using DirU   = std::underlying_type_t<Dir>;
    using Fault  = domain::common::MotorFault;
    using FaultU = std::underlying_type_t<Fault>;

    // =========================
    // Packed shared state in flaps_state_ (single allowed atomic)
    // bits:
    // 0: home limit
    // 1: end limit
    // 8..23: fault (16 bits)
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

    static constexpr std::uint32_t packFaultBits(Fault f) {
        return (static_cast<std::uint32_t>(static_cast<FaultU>(f)) & 0xFFFFu) << FAULT_SHIFT;
    }

    static constexpr std::uint32_t packFlapsBits(FlapsState s) {
        return (static_cast<std::uint32_t>(s) & 0x03u) << FLAPS_SHIFT;
    }

    static constexpr std::uint32_t packInitialState() {
        return packFaultBits(Fault::None) | packFlapsBits(FlapsState::CloseBoth);
    }

    FlapsState currentFlaps() const {
        const auto st = flaps_state_.load(std::memory_order_relaxed);
        return static_cast<FlapsState>((st & FLAPS_MASK) >> FLAPS_SHIFT);
    }

    void setFaultBits(Fault f) {
        const std::uint32_t newFault = packFaultBits(f);
        std::uint32_t old = flaps_state_.load(std::memory_order_relaxed);
        for (;;) {
            const std::uint32_t neu = (old & ~FAULT_MASK) | newFault;
            if (flaps_state_.compare_exchange_weak(old, neu,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed))
                return;
        }
    }

    void setFlapsBits(FlapsState s) {
        const std::uint32_t newFlaps = packFlapsBits(s);
        std::uint32_t old = flaps_state_.load(std::memory_order_relaxed);
        for (;;) {
            const std::uint32_t neu = (old & ~FLAPS_MASK) | newFlaps;
            if (flaps_state_.compare_exchange_weak(old, neu,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed))
                return;
        }
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

        std::uint32_t old = flaps_state_.load(std::memory_order_relaxed);
        for (;;) {
            const std::uint32_t neu = (old & ~(LIMIT_HOME_BIT | LIMIT_END_BIT)) | newLimitBits;
            if (flaps_state_.compare_exchange_weak(old, neu,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed))
                return;
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
            std::lock_guard lk(mtx_);
            copy = observers_;
        }
        for (auto* o : copy) fn(*o);
    }

    void notifyStopped()   { notifyAll([](auto& o){ o.onStopped(); }); }
    void notifyEmergency() { notifyAll([](auto& o){ o.onFault(domain::common::MotorFault::EmergencyStop); }); }
    void notifyFault(domain::common::MotorFault f) { notifyAll([&](auto& o){ o.onFault(f); }); }


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
                break;
            case FlapsState::OpenInput:
                lpt_.write(2, cfg_.byte_open_input_flap);
                break;
            case FlapsState::OpenOutput:
                lpt_.write(2, cfg_.byte_open_output_flap);
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

        // Применим начальное состояние флапсов сразу после открытия порта
        applyFlaps(currentFlaps());

        for (;;) {
            // Ждём события управления/остановки/клапанов
            {
                std::unique_lock lk(mtx_);
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
                runLoop(); // HOT PATH: no mutex inside
            }
        }

        // На выходе — безопасно гасим выходы
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
            log_.error("failed to open LPT port: {}", e.what());
            setFaultBits(Fault::DriverError);
            notifyFault(Fault::DriverError);
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

        setFaultBits(Fault::EmergencyStop);

        log_.error("!!! EMERGENCY STOP !!!");
        notifyEmergency();
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

    // immutable config (no runtime reconfigure)
    const G540LptMotorDriverConfig cfg_;
    const unsigned char begin_mask_;
    const unsigned char end_mask_;

    // worker-local cache to avoid CAS each iteration
    std::uint8_t last_limits_bits_{0};

    // mutex only for observers + cv wait (НЕ в runLoop)
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::thread worker_;
    std::vector<domain::ports::IMotorDriverObserver*> observers_;

    // ====== allowed atomics only ======
    std::atomic<bool> shutdown_{false};
    std::atomic<bool> requested_running_{false};
    std::atomic<bool> requested_emergency_{false};

    std::atomic<std::int64_t> half_period_ns_;
    std::atomic<DirU> direction_;

    std::atomic<bool> flaps_dirty_{false};
    std::atomic<std::uint32_t> flaps_state_;
};





// ==========================================================
// Driver
// ==========================================================

G540LptMotorDriver::G540LptMotorDriver(const G540LptMotorDriverPorts& ports,
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
    // Impl деструктор сам корректно остановит поток
}

void G540LptMotorDriver::start() {
    logger_.info("start()");
    impl_->start();
}

void G540LptMotorDriver::stop() {
    logger_.info("stop()");
    impl_->stop();
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

void G540LptMotorDriver::setDirection(domain::common::MotorDirection dir) {
    current_dir_ = dir;
    logger_.info("setDirection({})", dir);
    impl_->setDirection(dir);
}

domain::common::MotorLimitsState G540LptMotorDriver::limits() const {
    return impl_->limits();
}

domain::common::MotorFault G540LptMotorDriver::fault() const {
    return impl_->fault();
}

void G540LptMotorDriver::enableWatchdog(std::chrono::milliseconds) {
    // если нет — no-op
}

void G540LptMotorDriver::disableWatchdog() {
    // no-op
}

void G540LptMotorDriver::feedWatchdog() {
    // no-op
}


void G540LptMotorDriver::addObserver(domain::ports::IMotorDriverObserver& o) {
    impl_->addObserver(o);
}

void G540LptMotorDriver::removeObserver(domain::ports::IMotorDriverObserver& o) {
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
