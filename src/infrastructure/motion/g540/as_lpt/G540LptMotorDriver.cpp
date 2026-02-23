#include "G540LptMotorDriver.h"

#include "infrastructure/platform/lpt/LptPort.h"
#include "infrastructure/platform/sleep/sleep.h"

#include "../../../../domain/ports/drivers/motor/IMotorDriverObserver.h"
#include "../../../../domain/ports/drivers/valve/IValveDriverObserver.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>

#include "domain/core/drivers/motor/MotorError.h"

using namespace infra::motors;

namespace {

// ---------------- helpers ----------------
inline int clampHz(int hz, int minHz, int maxHz) {
    return std::max(minHz, std::min(maxHz, hz));
}

inline unsigned char invertBusyBit(unsigned char s) {
    return static_cast<unsigned char>(s ^ (1u << 7));
}

inline domain::common::MotorLimitsState toLimitsState(unsigned char state,
                                                      unsigned char beginMask,
                                                      unsigned char endMask)
{
    domain::common::MotorLimitsState out{};
    out.home = (state & beginMask) != 0;
    out.end  = (state & endMask)   != 0;
    return out;
}

inline bool canMove(domain::common::MotorDirection direction,
                    const domain::common::MotorLimitsState& limits)
{
    using domain::common::MotorDirection;
    switch (direction) {
        case MotorDirection::Forward:  return !limits.end;
        case MotorDirection::Backward: return !limits.home;
    }
    return false;
}


inline domain::common::MotorError makeStartError(const std::string& msg) {
    domain::common::MotorError e{};
    e.reason = msg;
    return e;
}

} // namespace

// ==========================================================
// Impl
// ==========================================================

struct G540LptMotorDriver::G540LptImpl {
    enum class DriverState : std::uint8_t {
        Fault,      // LPT не открыт, запуск невозможен
        Idle,       // готов, но не бежим
        Running,    // генерим шаги
        Emergency,  // аварийный стоп (одноразовая обработка)
        Shutdown    // завершение потока
    };

    using Dir  = domain::common::MotorDirection;
    using DirU = std::underlying_type_t<Dir>;

    explicit G540LptImpl(domain::ports::ILogger& logger,
                         const G540LptMotorDriverConfig& cfg)
        : log_(logger)
        , cfg_(cfg)
        , begin_mask_(static_cast<unsigned char>(1u << cfg_.bit_begin_limit_switch))
        , end_mask_(static_cast<unsigned char>(1u << cfg_.bit_end_limit_switch))
        , half_period_ns_(halfPeriodNsCount(clampHz(cfg_.min_freq_hz, cfg_.min_freq_hz, cfg_.max_freq_hz)))
        , direction_(static_cast<DirU>(Dir::Forward))
    {

    }

    ~G540LptImpl() { shutdownAndJoin(); }

    void initialize() {
        if (!openPortCtor()) {
            state_.store(DriverState::Fault, std::memory_order_relaxed);
        } else {
            state_.store(DriverState::Idle, std::memory_order_relaxed);
        }

        worker_ = std::thread([this]{ threadMain(); });
    }

    // ---------------- API ----------------

    bool start() {
        // Требование: вернуть false если LPT не открыт.
        if (!isPortOpen()) {
            notifyMotorStartFailed(last_start_error());
            return false;
        }

        DriverState expected = DriverState::Idle;
        if (state_.compare_exchange_strong(expected, DriverState::Running,
                                           std::memory_order_relaxed,
                                           std::memory_order_relaxed))
        {
            cv_.notify_all();
            return true;
        }

        // уже Running -> ok
        if (expected == DriverState::Running) return true;

        // Emergency/Fault/Shutdown -> нельзя
        notifyMotorStartFailed(makeStartError("motor start rejected by driver state"));
        return false;
    }

    void stop() {
        // stop переводит в Idle; worker сам отработает и пошлёт onMotorStopped()
        auto s = state_.load(std::memory_order_relaxed);
        if (s == DriverState::Running) {
            state_.store(DriverState::Idle, std::memory_order_relaxed);
        }
        cv_.notify_all();
    }

    bool isRunning() const {
        return state_.load(std::memory_order_relaxed) == DriverState::Running;
    }

    void abort() {
        // аварийный стоп имеет приоритет
        auto s = state_.load(std::memory_order_relaxed);
        if (s == DriverState::Shutdown) return;
        state_.store(DriverState::Emergency, std::memory_order_relaxed);
        cv_.notify_all();
    }

    bool isPortOpen() const {
        return port_ok_;
    }

    void setDirection(Dir d) {
        const auto old = direction_.exchange(static_cast<DirU>(d), std::memory_order_relaxed);
        if (old != static_cast<DirU>(d)) {
            notifyMotorDirectionChanged(d);
        }
    }

    void setHz(int hz) {
        hz = clampHz(hz, cfg_.min_freq_hz, cfg_.max_freq_hz);
        half_period_ns_.store(halfPeriodNsCount(hz), std::memory_order_relaxed);
    }

    void setFlaps(FlapsState s) {
        flaps_cmd_.store(static_cast<std::uint8_t>(s), std::memory_order_relaxed);
        flaps_seq_.fetch_add(1, std::memory_order_release); // publish point
        cv_.notify_all();
    }

    domain::common::MotorLimitsState limits() const {
        // лимиты читаем через last_limits_bits_ (atomic не нужен, это просто snapshot)
        // но потоков несколько => сделаем атомиком байт, чтобы чтение было корректным.
        const auto b = last_limits_bits_.load(std::memory_order_relaxed);
        domain::common::MotorLimitsState out{};
        out.home = (b & 0x01u) != 0;
        out.end  = (b & 0x02u) != 0;
        return out;
    }

    // observers
    void addObserver(domain::ports::IMotorDriverObserver& o) {
        std::lock_guard lk(obs_mtx_);
        motor_observers_.push_back(&o);
    }

    void removeObserver(domain::ports::IMotorDriverObserver& o) {
        std::lock_guard lk(obs_mtx_);
        motor_observers_.erase(std::remove(motor_observers_.begin(), motor_observers_.end(), &o),
                               motor_observers_.end());
    }

    void addObserver(domain::ports::IValveDriverObserver& o) {
        std::lock_guard lk(valve_obs_mtx_);
        valve_observers_.push_back(&o);
    }

    void removeObserver(domain::ports::IValveDriverObserver& o) {
        std::lock_guard lk(valve_obs_mtx_);
        valve_observers_.erase(std::remove(valve_observers_.begin(), valve_observers_.end(), &o),
                               valve_observers_.end());
    }

private:
    // ---------------- constants ----------------
    static std::int64_t halfPeriodNsCount(int hz) {
        if (hz <= 0) hz = 1;
        const std::int64_t denom = static_cast<std::int64_t>(hz) * 2;
        std::int64_t ns = 1'000'000'000LL / denom;
        if (ns <= 0) ns = 1;
        return ns;
    }

    // ---------------- notification helpers ----------------
    template<class Fn>
    void notifyAllMotor(Fn&& fn) {
        std::vector<domain::ports::IMotorDriverObserver*> copy;
        {
            std::lock_guard lk(obs_mtx_);
            copy = motor_observers_;
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

    void notifyMotorStarted() { notifyAllMotor([](auto& o){ o.onMotorStarted(); }); }
    void notifyMotorStopped() { notifyAllMotor([](auto& o){ o.onMotorStopped(); }); }

    void notifyMotorStartFailed(const domain::common::MotorError& e) {
        notifyAllMotor([&](auto& o){ o.onMotorStartFailed(e); });
    }

    void notifyMotorLimitsChanged(domain::common::MotorLimitsState s) {
        notifyAllMotor([s](auto& o){ o.onMotorLimitsStateChanged(s); });
    }

    void notifyMotorDirectionChanged(Dir d) {
        notifyAllMotor([d](auto& o){ o.onMotorDirectionChanged(d); });
    }

    void notifyInputFlapOpened()  { notifyAllValve([](auto& o){ o.onInputFlapOpened(); }); }
    void notifyOutputFlapOpened() { notifyAllValve([](auto& o){ o.onOutputFlapOpened(); }); }
    void notifyFlapsClosed()      { notifyAllValve([](auto& o){ o.onFlapsClosed(); }); }

    // ---------------- LPT low level ----------------
    unsigned char readState() {
        return invertBusyBit(lpt_.read(1));
    }

    void writeNeutral() {
        // lpt_.write(0, 0);
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
        const unsigned char b1 = static_cast<unsigned char>((d * 2 + 0) << shift);
        const unsigned char b2 = static_cast<unsigned char>((d * 2 + 1) << shift);

        lpt_.write(0, b1);
        platform::precise_sleep(halfPeriod);

        lpt_.write(0, b2);
        platform::precise_sleep(halfPeriod);
    }

    // ---------------- port open in ctor ----------------
    bool openPortCtor() {
        try {
            lpt_.open(cfg_.lpt_port);
            port_ok_ = true;
            log_.info("LPT port opened");
            return true;
        } catch (const std::exception& e) {
            {
                std::lock_guard lk(start_err_mtx_);
                start_error_ = makeStartError(e.what());
            }
            port_ok_ = false;
            log_.error("failed to open LPT port: {}", e.what());
            return false;
        } catch (...) {
            {
                std::lock_guard lk(start_err_mtx_);
                start_error_ = makeStartError("unknown error while opening LPT port");
            }
            port_ok_ = false;
            log_.error("failed to open LPT port: unknown error");
            return false;
        }
    }

    domain::common::MotorError last_start_error() const {
        std::lock_guard lk(start_err_mtx_);
        return start_error_;
    }

    // ---------------- limits publish (changed-only) ----------------
    void publishLimitsIfChanged(bool home, bool end) {
        std::uint8_t bits = 0;
        if (home) bits |= 0x01u;
        if (end)  bits |= 0x02u;

        const auto prev = last_limits_bits_.load(std::memory_order_relaxed);
        if (bits == prev) return;

        last_limits_bits_.store(bits, std::memory_order_relaxed);

        domain::common::MotorLimitsState s{};
        s.home = home;
        s.end  = end;
        notifyMotorLimitsChanged(s);
    }

    // ---------------- flaps pending ----------------
    bool popFlapsDirty(FlapsState& out) {
        const auto seq = flaps_seq_.load(std::memory_order_acquire);
        if (seq == flaps_seen_seq_) return false;

        out = static_cast<FlapsState>(flaps_cmd_.load(std::memory_order_relaxed));
        flaps_seen_seq_ = seq;
        return true;
    }

    // ---------------- thread ----------------
    void shutdownAndJoin() {
        state_.store(DriverState::Shutdown, std::memory_order_relaxed);
        cv_.notify_all();
        if (worker_.joinable()) worker_.join();

        if (port_ok_) {
            // worker уже не трогает железо => можно закрыть тут
            try {
                lpt_.close();
            } catch (...) {}
        }
    }

    void threadMain() {
        log_.info("G540Lpt worker started");

        if (!port_ok_) {
            // В Fault мы не трогаем железо. Ждём Shutdown.
            for (;;) {
                std::unique_lock lk(cv_mtx_);
                cv_.wait(lk, [&]{ return state_.load(std::memory_order_relaxed) == DriverState::Shutdown; });
                break;
            }
            log_.info("G540Lpt worker stopped (Fault)");
            return;
        }

        flaps_seen_seq_ = flaps_seq_.load(std::memory_order_acquire);

        while (true) {
            auto s = state_.load(std::memory_order_relaxed);
            if (s == DriverState::Shutdown) break;

            if (s == DriverState::Emergency) {
                handleEmergency();
                // после аварии остаёмся в Idle
                state_.store(DriverState::Idle, std::memory_order_relaxed);
                continue;
            }

            // флапсы должны работать даже когда Idle
            FlapsState fs{};
            if (popFlapsDirty(fs)) {
                applyFlaps(fs);
            }

            if (s == DriverState::Idle) {
                // спим до событий
                std::unique_lock lk(cv_mtx_);
                cv_.wait(lk, [&]{
                    const auto st = state_.load(std::memory_order_acquire);
                    if (st == DriverState::Shutdown)  return true;
                    if (st == DriverState::Running)   return true;
                    if (st == DriverState::Emergency) return true;
                    return flaps_seq_.load(std::memory_order_acquire) != flaps_seen_seq_;
                });
                continue;
            }

            if (s == DriverState::Running) {
                notifyMotorStarted();
                runLoop();
                // runLoop выходит только когда state != Running
                notifyMotorStopped();
                continue;
            }

            if (s == DriverState::Fault) {
                // теоретически сюда не попадём (порт_ok_==true), но пусть будет
                std::unique_lock lk(cv_mtx_);
                cv_.wait(lk, [&]{ return state_.load(std::memory_order_relaxed) == DriverState::Shutdown; });
                break;
            }
        }

        // shutdown
        writeNeutral();
        log_.info("G540Lpt worker stopped");
    }

    void handleEmergency() {
        writeNeutral();
        log_.error("!!! EMERGENCY STOP !!!");

        const auto st  = readState();
        const auto lim = toLimitsState(st, begin_mask_, end_mask_);
        publishLimitsIfChanged(lim.home, lim.end);
    }

    // HOT PATH: no mutex
    void runLoop() {
        for (;;) {
            const auto st = state_.load(std::memory_order_relaxed);
            if (st != DriverState::Running) {
                writeNeutral();
                return;
            }

            // флапсы на лету
            FlapsState fs{};
            if (popFlapsDirty(fs)) {
                applyFlaps(fs);
            }

            const auto dir = static_cast<Dir>(direction_.load(std::memory_order_relaxed));
            const auto hp  = std::chrono::nanoseconds(half_period_ns_.load(std::memory_order_relaxed));

            const auto raw = readState();
            const auto lim = toLimitsState(raw, begin_mask_, end_mask_);
            publishLimitsIfChanged(lim.home, lim.end);

            if (!canMove(dir, lim)) {
                platform::sleep(std::chrono::milliseconds(25));
                continue;
            }

            stepOnce(dir, hp);

            // авария имеет приоритет, даже если Running (проверяем после шага)
            if (state_.load(std::memory_order_relaxed) == DriverState::Emergency) {
                handleEmergency();
                state_.store(DriverState::Idle, std::memory_order_relaxed);
                writeNeutral();
                return;
            }
        }
    }

private:
    fmt::Logger log_;

    // hardware (worker only after ctor)
    platform::LptPort lpt_;
    bool port_ok_{false};

    // immutable config
    const G540LptMotorDriverConfig cfg_;
    const unsigned char begin_mask_;
    const unsigned char end_mask_;

    // explicit state machine (the only atomic for lifecycle)
    std::atomic<DriverState> state_{DriverState::Fault};

    // motion config (atomics allowed, не про lifecycle)
    std::atomic<std::int64_t> half_period_ns_;
    std::atomic<DirU> direction_;

    // limits snapshot (changed-only)
    std::atomic<std::uint8_t> last_limits_bits_{0};

    // observers
    mutable std::mutex obs_mtx_;
    std::vector<domain::ports::IMotorDriverObserver*> motor_observers_;

    mutable std::mutex valve_obs_mtx_;
    std::vector<domain::ports::IValveDriverObserver*> valve_observers_;

    // start error
    mutable std::mutex start_err_mtx_;
    domain::common::MotorError start_error_{ makeStartError("lpt not opened") };

    // flaps (lock-free publish)
    std::atomic<std::uint8_t>  flaps_cmd_{ static_cast<std::uint8_t>(FlapsState::CloseBoth) };
    std::atomic<std::uint32_t> flaps_seq_{ 0 };     // producer increments on every setFlaps()
    std::uint32_t              flaps_seen_seq_{ 0 };// consumer (worker thread only)

    // cv (wait only)
    mutable std::mutex cv_mtx_;
    std::condition_variable cv_;

    std::thread worker_;
};

// ==========================================================
// Driver
// ==========================================================

G540LptMotorDriver::G540LptMotorDriver(const MotorDriverPorts& ports,
                                       const G540LptMotorDriverConfig& config)
    : logger_(ports.logger)
    , impl_(std::make_unique<G540LptImpl>(ports.logger, config))
    , ports_(ports)
    , config_(config)
    , current_hz_(clampHz(config.min_freq_hz, config.min_freq_hz, config.max_freq_hz))
    , current_dir_(domain::common::MotorDirection::Forward)
{
    logger_.info("G540LptMotorDriver constructed");

    impl_->setDirection(current_dir_.load(std::memory_order_relaxed));
    impl_->setHz(current_hz_.load(std::memory_order_relaxed));
}

G540LptMotorDriver::~G540LptMotorDriver() {
    logger_.info("G540LptMotorDriver destructing");
}

void G540LptMotorDriver::initialize() {
    impl_->initialize();
}

bool G540LptMotorDriver::start() {
    logger_.info("start()");
    return impl_->start();
}

void G540LptMotorDriver::stop() {
    logger_.info("stop()");
    impl_->stop();
}

bool G540LptMotorDriver::isRunning() const {
    return impl_->isRunning();
}

void G540LptMotorDriver::abort() {
    logger_.error("abort() -> emergency");
    impl_->abort();
}

void G540LptMotorDriver::setFrequency(int hz) {
    hz = clampHz(hz, config_.min_freq_hz, config_.max_freq_hz);
    current_hz_.store(hz, std::memory_order_relaxed);
    logger_.info("setFrequency({})", hz);
    impl_->setHz(hz);
}

int G540LptMotorDriver::frequency() const {
    return current_hz_.load(std::memory_order_relaxed);
}

domain::common::MotorFrequencyLimits G540LptMotorDriver::frequencyLimits() const {
    return { config_.min_freq_hz, config_.max_freq_hz };
}

void G540LptMotorDriver::setDirection(domain::common::MotorDirection dir) {
    current_dir_.store(dir, std::memory_order_relaxed);
    logger_.info("setDirection({})", dir);
    impl_->setDirection(dir);
}

domain::common::MotorDirection G540LptMotorDriver::direction() const {
    return current_dir_.load(std::memory_order_relaxed);
}

domain::common::MotorLimitsState G540LptMotorDriver::limits() const {
    return impl_->limits();
}

void G540LptMotorDriver::addObserver(domain::ports::IMotorDriverObserver& o) {
    impl_->addObserver(o);
}

void G540LptMotorDriver::removeObserver(domain::ports::IMotorDriverObserver& o) {
    impl_->removeObserver(o);
}

bool G540LptMotorDriver::openInputFlap() {
    logger_.info("openInputFlap()");
    if (!impl_->isPortOpen()) return false;
    setFlapsState(FlapsState::OpenInput);
    return true;
}

bool G540LptMotorDriver::openOutputFlap() {
    logger_.info("openOutputFlap()");
    if (!impl_->isPortOpen()) return false;
    setFlapsState(FlapsState::OpenOutput);
    return true;
}

bool G540LptMotorDriver::closeFlaps() {
    logger_.info("closeFlaps()");
    if (!impl_->isPortOpen()) return false;
    setFlapsState(FlapsState::CloseBoth);
    return true;
}

void G540LptMotorDriver::addObserver(domain::ports::IValveDriverObserver& o) {
    impl_->addObserver(o);
}

void G540LptMotorDriver::removeObserver(domain::ports::IValveDriverObserver& o) {
    impl_->removeObserver(o);
}

void G540LptMotorDriver::setFlapsState(FlapsState s) {
    impl_->setFlaps(s);
}
