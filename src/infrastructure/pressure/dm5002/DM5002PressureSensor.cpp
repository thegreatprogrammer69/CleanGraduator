#include "DM5002PressureSensor.h"
#include <stdexcept>
#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "domain/ports/clock/IClock.h"
#include "domain/core/pressure/PressurePacket.h"
#include "domain/core/pressure/PressureSourceError.h"
#include "infrastructure/platform/sleep/sleep.h"
#include "infrastructure/platform/com/ComPort.h"

namespace infra::pressure {

constexpr int responseLength = 19;

struct DM5002PressureSensor::DM5002PressureSensorImpl {
    explicit DM5002PressureSensorImpl() {}

    unsigned char requestBytes[12] = {
        0xFF, 0xFF, 0xFF, 0x82,
        0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x01, 0x00, 0x83
    };
    unsigned char responseBytes[64] = {};

    std::atomic<bool> stopped{true};
    std::thread worker;

    platform::ComPort com_port;
};

DM5002PressureSensor::DM5002PressureSensor(PressureSourcePorts ports, DM5002PressureSensorConfig config)
    : impl_(std::make_unique<DM5002PressureSensorImpl>())
    , config_(config)
    , logger_(ports.logger)
    , clock_(ports.clock)
{
    logger_.info("constructor called");
}

DM5002PressureSensor::~DM5002PressureSensor() {
    logger_.info("destructor called, stopping...");
    stop();
}

bool DM5002PressureSensor::start() {
    using namespace std::chrono;

    logger_.info("DM5002PressureSensor::start() called");

    const bool wasStopped = impl_->stopped.load(std::memory_order_acquire);
    logger_.info("current stopped state = {}", wasStopped);

    if (!wasStopped) {
        logger_.warn("start() aborted: worker already running");
        return false;
    }

    const bool wasJoinable = impl_->worker.joinable();
    logger_.info("worker.joinable() = {}", wasJoinable);

    if (wasJoinable) {
        logger_.warn("joining previous worker thread");
        impl_->worker.join();
        logger_.info("previous worker joined");
    }

    logger_.info("setting stopped = false");
    impl_->stopped.store(false, std::memory_order_release);

    try {
        logger_.info("creating worker thread");
        impl_->worker = std::thread(&DM5002PressureSensor::run, this);
        logger_.info("worker thread successfully created");
    } catch (const std::exception& ex) {
        logger_.error("failed to create worker thread: {}", ex.what());
        impl_->stopped.store(true, std::memory_order_release);
        return false;
    } catch (...) {
        logger_.error("failed to create worker thread: unknown exception");
        impl_->stopped.store(true, std::memory_order_release);
        return false;
    }

    logger_.info("DM5002PressureSensor::start() finished successfully");
    return true;
}


void DM5002PressureSensor::stop() {
    using namespace std::chrono;
    if (impl_->stopped) return;
    impl_->stopped.store(true, std::memory_order_release);
    logger_.info("waiting for worker to stop");
    if (impl_->worker.joinable()) impl_->worker.join();
    logger_.info("worker stopped");
}

bool DM5002PressureSensor::isRunning() const noexcept {
    return !impl_->stopped.load(std::memory_order_acquire);
}

void DM5002PressureSensor::addObserver(domain::ports::IPressureSourceObserver &observer) {
    notifier_.addObserver(observer);
}

void DM5002PressureSensor::removeObserver(domain::ports::IPressureSourceObserver &observer) {
    notifier_.removeObserver(observer);
}

void DM5002PressureSensor::run() {
    using namespace std::chrono;

    logger_.info("worker thread started");

    logger_.info("opening COM port");

    try {
        impl_->com_port.open(config_.com_port);
    }
    catch (std::exception &e) {
        logger_.error("failed to open COM port: {}", e.what());
        notifier_.notifyOpenFailed({logger_.lastError()});
        return;
    }

    notifier_.notifyOpened();

    int invalid_count = 0;

    while (!impl_->stopped.load(std::memory_order_acquire)) {
        platform::sleep(90ms);

        const auto result = readPressure();

        if (!result.valid) {
            invalid_count += 1;
            logger_.warn("read failed (attempt {}/3)", invalid_count);
            if (invalid_count >= 3) {
                logger_.error("stopped, {} failures, see logs", invalid_count);
                break;
            }
            continue;
        }

        if (invalid_count > 0) {
            logger_.info("communication restored after {} failed attempts", invalid_count);
            invalid_count = 0;
        }

        logger_.info("pressure sample: value={} Pa, ts={}",result.pressure, result.time_point);

        domain::common::PressurePacket packet;
        packet.timestamp = result.time_point;
        packet.pressure = result.pressure;

        notifier_.notifyPressure(packet);
    }

    logger_.info("closing COM port");
    impl_->com_port.close();

    logger_.info("worker thread stopped");

    domain::common::PressureSourceError err{ logger_.lastError() };
    notifier_.notifyClosed(err);
}

DM5002PressureSensor::ReadResult DM5002PressureSensor::readPressure() {
    ReadResult result;

    // =================================
    // 1. Отправка запроса

    logger_.info("reading pressure");

    const auto bytesWritten = impl_->com_port.write(impl_->requestBytes, sizeof(impl_->requestBytes));
    if (bytesWritten != sizeof(impl_->requestBytes)) {
        logger_.warn(
            "pressure request write failed: written {} of {} bytes",
            bytesWritten, sizeof(impl_->requestBytes));
        return result;
    }

    // =================================
    // 2. Чтение ответа
    const auto bytesRead = impl_->com_port.read(impl_->responseBytes, responseLength);

    if (bytesRead != responseLength) {
        logger_.warn(
            "pressure response read failed: read {} of {} bytes",
            bytesRead, responseLength);
        return result;
    }

    result.time_point = clock_.now();

    logger_.info("pressure response received");

    // =================================
    // 3. Извлечение значения давления
    unsigned char pressureBytes[4] = {
        impl_->responseBytes[17],
        impl_->responseBytes[16],
        impl_->responseBytes[15],
        impl_->responseBytes[14]
    };

    float pressure = 0.0F;
    std::memcpy(&pressure, pressureBytes, sizeof(float));

    if (!std::isfinite(pressure)) {
        logger_.error(
            "invalid pressure value received (NaN or Inf), raw bytes: {:X} {:X} {:X} {:X}",
            static_cast<int>(pressureBytes[3]),
            static_cast<int>(pressureBytes[2]),
            static_cast<int>(pressureBytes[1]),
            static_cast<int>(pressureBytes[0])
        );
        return result;
    }

    // =================================
    // 4. Извлечение единицы измерения и конвертация в Па
    const unsigned char unitByte = impl_->responseBytes[13];

    switch (unitByte) {
        case 1:
            result.pressure = domain::common::Pressure::fromKgfCm2(pressure);
            break;
        case 2:
            result.pressure = domain::common::Pressure::fromMPa(pressure);
            break;
        case 3:
            result.pressure = domain::common::Pressure::fromKPa(pressure);
            break;
        case 4:
            result.pressure = domain::common::Pressure::fromPa(pressure);
            break;
        case 5:
            result.pressure = domain::common::Pressure::fromKgfM2(pressure);
            break;
        case 6:
            result.pressure = domain::common::Pressure::fromAtm(pressure);
            break;
        case 7:
            result.pressure = domain::common::Pressure::fromMmHg(pressure);
            break;
        case 8:
            result.pressure = domain::common::Pressure::fromMmH2O(pressure);
            break;
        case 9:
            result.pressure = domain::common::Pressure::fromBar(pressure);
            break;
        default:
            return result;
    }

    logger_.info("pressure: {}", result.pressure);

    result.valid = true;
    return result;
}

}
