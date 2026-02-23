#include "DM5002PressureSensor.h"

#include <atomic>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>
#include <stdexcept>

#include "domain/ports/clock/IClock.h"
#include "domain/core/pressure/PressurePacket.h"
#include "domain/core/pressure/PressureSourceError.h"

#include "infrastructure/platform/sleep/sleep.h"
#include "infrastructure/platform/com/ComPort.h"

namespace infra::pressure {

namespace {

constexpr std::size_t kMaxResponseBytes = 64;

struct ProtocolLayout {
    std::vector<unsigned char> request;
    std::size_t response_length{};
    std::size_t unit_index{};
    // indices in LE order for memcpy into float on little-endian host:
    // b[0]=LSB ... b[3]=MSB
    std::array<std::size_t, 4> float_le_indices{};
};

ProtocolLayout layoutFor(DM5002PressureSensorConfig::Protocol p) {
    switch (p) {
        case DM5002PressureSensorConfig::Protocol::Standard:
            return ProtocolLayout{
                /*request*/ { 0xFF, 0xFF, 0xFF, 0x82,
                             0xFF, 0xFF, 0xFF, 0xFF,
                             0x00, 0x01, 0x00, 0x83 },
                /*response_length*/ 19,
                /*unit_index*/ 13,
                /*float_indices*/ { 17, 16, 15, 14 }
            };
        case DM5002PressureSensorConfig::Protocol::RF:
            return ProtocolLayout{
                /*request*/ { 0x00, 0x01 },
                /*response_length*/ 5,
                /*unit_index*/ 0,
                /*float_indices*/ { 4, 3, 2, 1 }
            };
        default:
            throw std::invalid_argument("DM5002PressureSensorConfig::Protocol: unsupported value");
    }
}

std::chrono::milliseconds pollIntervalFromHz(int hz) {
    if (hz <= 0) hz = 10;
    // floor division is fine; for hz>1000 => 0ms would be bad
    const int ms = std::max(1, 1000 / hz);
    return std::chrono::milliseconds(ms);
}

bool convertToPressure(unsigned char unitByte, float value, domain::common::Pressure& out) {
    switch (unitByte) {
        case 1: out = domain::common::Pressure::fromKgfCm2(value); return true;
        case 2: out = domain::common::Pressure::fromMPa(value);    return true;
        case 3: out = domain::common::Pressure::fromKPa(value);    return true;
        case 4: out = domain::common::Pressure::fromPa(value);     return true;
        case 5: out = domain::common::Pressure::fromKgfM2(value);  return true;
        case 6: out = domain::common::Pressure::fromAtm(value);    return true;
        case 7: out = domain::common::Pressure::fromMmHg(value);   return true;
        case 8: out = domain::common::Pressure::fromMmH2O(value);  return true;
        case 9: out = domain::common::Pressure::fromBar(value);    return true;
        default: return false;
    }
}

} // namespace

struct DM5002PressureSensor::Impl {
    explicit Impl(DM5002PressureSensorConfig cfg)
        : layout(layoutFor(cfg.protocol))
        , poll_interval(pollIntervalFromHz(cfg.poll_rate))
    {
        if (layout.response_length == 0 || layout.response_length > kMaxResponseBytes) {
            throw std::invalid_argument("DM5002PressureSensor: invalid response length");
        }
        if (layout.unit_index >= layout.response_length) {
            throw std::invalid_argument("DM5002PressureSensor: unit_index out of bounds");
        }
        for (auto idx : layout.float_le_indices) {
            if (idx >= layout.response_length) {
                throw std::invalid_argument("DM5002PressureSensor: float byte index out of bounds");
            }
        }
        if (layout.request.empty()) {
            throw std::invalid_argument("DM5002PressureSensor: request bytes empty");
        }
    }

    ProtocolLayout layout;
    std::chrono::milliseconds poll_interval{100};

    std::array<unsigned char, kMaxResponseBytes> responseBytes{};

    std::atomic<bool> stop_requested{true};
    std::atomic<bool> running{false};
    std::thread worker;
    std::mutex lifecycle_mtx;

    platform::ComPort com_port;
};

DM5002PressureSensor::DM5002PressureSensor(PressureSourcePorts ports, DM5002PressureSensorConfig config)
    : impl_(std::make_unique<Impl>(config))
    , config_(std::move(config))
    , logger_(ports.logger)
    , clock_(ports.clock)
{
    logger_.info("DM5002PressureSensor constructed (protocol={}, poll_rate={} Hz, port={})",
                 static_cast<int>(config_.protocol), config_.poll_rate, config_.com_port);
}

DM5002PressureSensor::~DM5002PressureSensor() {
    logger_.info("DM5002PressureSensor destructor: stopping...");
    stop();
}

bool DM5002PressureSensor::start() {
    std::lock_guard<std::mutex> lk(impl_->lifecycle_mtx);

    logger_.info("DM5002PressureSensor::start()");

    if (impl_->running.load(std::memory_order_acquire)) {
        logger_.warn("start() aborted: already running");
        return false;
    }

    if (impl_->worker.joinable()) {
        logger_.warn("joining previous worker thread");
        impl_->worker.join();
    }

    impl_->stop_requested.store(false, std::memory_order_release);
    impl_->running.store(true, std::memory_order_release);

    try {
        impl_->worker = std::thread(&DM5002PressureSensor::run, this);
    } catch (const std::exception& ex) {
        logger_.error("failed to create worker thread: {}", ex.what());
        impl_->stop_requested.store(true, std::memory_order_release);
        impl_->running.store(false, std::memory_order_release);
        return false;
    } catch (...) {
        logger_.error("failed to create worker thread: unknown exception");
        impl_->stop_requested.store(true, std::memory_order_release);
        impl_->running.store(false, std::memory_order_release);
        return false;
    }

    return true;
}

void DM5002PressureSensor::stop() {
    std::lock_guard<std::mutex> lk(impl_->lifecycle_mtx);

    if (!impl_->running.load(std::memory_order_acquire) && !impl_->worker.joinable()) {
        return;
    }

    logger_.info("DM5002PressureSensor::stop(): requesting stop");
    impl_->stop_requested.store(true, std::memory_order_release);

    if (impl_->worker.joinable()) {
        impl_->worker.join();
    }

    impl_->running.store(false, std::memory_order_release);
    logger_.info("DM5002PressureSensor::stop(): stopped");
}

bool DM5002PressureSensor::isRunning() const noexcept {
    return impl_->running.load(std::memory_order_acquire) &&
           !impl_->stop_requested.load(std::memory_order_acquire);
}

void DM5002PressureSensor::addObserver(domain::ports::IPressureSourceObserver& observer) {
    notifier_.addObserver(observer);
}

void DM5002PressureSensor::removeObserver(domain::ports::IPressureSourceObserver& observer) {
    notifier_.removeObserver(observer);
}

void DM5002PressureSensor::run() {
    using namespace std::chrono;

    logger_.info("worker thread started");

    auto finish = [&]() {
        impl_->running.store(false, std::memory_order_release);
        logger_.info("worker thread exiting");
    };

    // --- open COM port
    try {
        logger_.info("opening COM port {}", config_.com_port);
        impl_->com_port.open(config_.com_port);
    } catch (const std::exception& e) {
        logger_.error("failed to open COM port: {}", e.what());
        notifier_.notifyOpenFailed({ logger_.lastError() });
        finish();
        return;
    } catch (...) {
        logger_.error("failed to open COM port: unknown exception");
        notifier_.notifyOpenFailed({ logger_.lastError() });
        finish();
        return;
    }

    notifier_.notifyOpened();

    int invalid_count = 0;

    try {
        while (!impl_->stop_requested.load(std::memory_order_acquire)) {
            platform::sleep(impl_->poll_interval);

            const auto result = readPressure();
            if (!result.valid) {
                ++invalid_count;
                logger_.warn("read failed (attempt {}/{})", invalid_count, config_.max_consecutive_failures);

                if (config_.max_consecutive_failures > 0 &&
                    invalid_count >= config_.max_consecutive_failures) {
                    logger_.error("stopping after {} consecutive failures", invalid_count);
                    break;
                }
                continue;
            }

            if (invalid_count > 0) {
                logger_.info("communication restored after {} failed attempts", invalid_count);
                invalid_count = 0;
            }

            domain::common::PressurePacket packet;
            packet.timestamp = result.time_point;
            packet.pressure  = result.pressure;

            notifier_.notifyPressure(packet);
        }
    } catch (const std::exception& e) {
        logger_.error("worker loop exception: {}", e.what());
    } catch (...) {
        logger_.error("worker loop exception: unknown");
    }

    // --- close COM port
    logger_.info("closing COM port");
    try {
        impl_->com_port.close();
    } catch (const std::exception& e) {
        logger_.warn("COM port close threw: {}", e.what());
    } catch (...) {
        logger_.warn("COM port close threw: unknown");
    }

    domain::common::PressureSourceError err{ logger_.lastError() };
    notifier_.notifyClosed(err);

    finish();
}

DM5002PressureSensor::ReadResult DM5002PressureSensor::readPressure() {
    ReadResult result;

    // 1) write request
    const auto& req = impl_->layout.request;
    const auto bytesWritten = impl_->com_port.write(req.data(), req.size());
    if (bytesWritten != req.size()) {
        logger_.warn("pressure request write failed: written {} of {} bytes",
                     bytesWritten, req.size());
        return result;
    }

    // 2) read response
    const auto respLen = impl_->layout.response_length;
    const auto bytesRead = impl_->com_port.read(impl_->responseBytes.data(), static_cast<int>(respLen));
    if (static_cast<std::size_t>(bytesRead) != respLen) {
        logger_.warn("pressure response read failed: read {} of {} bytes",
                     bytesRead, respLen);
        return result;
    }

    result.time_point = clock_.now();

    // 3) extract float (LE byte order for memcpy)
    unsigned char b[4] = {
        impl_->responseBytes[impl_->layout.float_le_indices[0]],
        impl_->responseBytes[impl_->layout.float_le_indices[1]],
        impl_->responseBytes[impl_->layout.float_le_indices[2]],
        impl_->responseBytes[impl_->layout.float_le_indices[3]],
    };

    float pressureValue = 0.0f;
    std::memcpy(&pressureValue, b, sizeof(float));

    if (!std::isfinite(pressureValue)) {
        logger_.error("invalid pressure value (NaN/Inf), raw bytes: {:X} {:X} {:X} {:X}",
                      static_cast<int>(b[3]), static_cast<int>(b[2]),
                      static_cast<int>(b[1]), static_cast<int>(b[0]));
        return result;
    }

    // 4) unit conversion
    const unsigned char unitByte = impl_->responseBytes[impl_->layout.unit_index];
    if (!convertToPressure(unitByte, pressureValue, result.pressure)) {
        logger_.error("unknown pressure unit byte: {}", static_cast<int>(unitByte));
        return result;
    }

    result.valid = true;
    return result;
}

} // namespace infra::pressure