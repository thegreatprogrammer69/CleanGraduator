#include "DM5002RFPressureSensor.h"
#include <stdexcept>
#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "domain/ports/outbound/IClock.h"
#include "infrastructure/platform/sleep/sleep.h"
#include "infrastructure/platform/com/ComPort.h"

namespace infra::hardware {

constexpr int responseLength = 5;

struct DM5002RFPressureSensor::DM5002RFPressureSensorImpl {
    explicit DM5002RFPressureSensorImpl(const std::string& com_port)
        : com_port(com_port)
    {}

    unsigned char requestBytes[2] = {0, 1};
    unsigned char responseBytes[64] = {};
    std::mutex sinks_mutex;
    std::vector<domain::ports::IPressureSink*> sinks;

    std::atomic<bool> stopped{true};
    std::thread worker;

    platform::ComPort com_port;
};

DM5002RFPressureSensor::DM5002RFPressureSensor(const PressureSensorPorts &ports, const DM5002RFConfig& config)
    : impl_(std::make_unique<DM5002RFPressureSensorImpl>(config.com_port))
    , ports_(ports)
    , config_(config)
    , logger_(ports.logger)
{
}

DM5002RFPressureSensor::~DM5002RFPressureSensor() {
    stop();
}

bool DM5002RFPressureSensor::start() {
    using namespace std::chrono;
    if (!impl_->stopped) return false;

    logger_.info("starting worker thread");

    if (impl_->worker.joinable()) {
        logger_.warn("joining previous worker");
        impl_->worker.join();
    }

    impl_->stopped.store(false, std::memory_order_release);
    impl_->worker = std::thread(&DM5002RFPressureSensor::run, this);

    logger_.info("worker started");
    return true;
}

void DM5002RFPressureSensor::stop() {
    using namespace std::chrono;
    if (impl_->stopped) return;
    impl_->stopped.store(true, std::memory_order_release);
    logger_.info("waiting for worker to stop");
    if (impl_->worker.joinable()) impl_->worker.join();
    logger_.info("worker stopped");
}

void DM5002RFPressureSensor::addSink(domain::ports::IPressureSink &sink) {
    std::lock_guard lock(impl_->sinks_mutex);
    impl_->sinks.push_back(&sink);
}

void DM5002RFPressureSensor::removeSink(domain::ports::IPressureSink &sink) {
    std::lock_guard lock(impl_->sinks_mutex);
    impl_->sinks.erase(
        std::remove(impl_->sinks.begin(), impl_->sinks.end(), &sink),
        impl_->sinks.end()
    );
}

void DM5002RFPressureSensor::run() {
    using namespace std::chrono;
    logger_.info("worker thread started");
    while (!impl_->stopped.load(std::memory_order_acquire)) {
        platform::sleep(90ms);
        const auto result = readPressure();
        if (!result.valid) continue;

        std::vector<domain::ports::IPressureSink*> sinksCopy; {
            std::lock_guard lock(impl_->sinks_mutex);
            sinksCopy = impl_->sinks;
        }
        for (auto* s : sinksCopy) {
            if (s) s->onPressure(result.time_point, result.pressure);
        }
    }
    logger_.info("worker thread stopped");
}

DM5002RFPressureSensor::ReadResult DM5002RFPressureSensor::readPressure() {
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

    result.time_point = ports_.session_clock.now();

    logger_.info("pressure response received");

    // =================================
    // 3. Извлечение значения давления
    unsigned char pressureBytes[4] = {
        impl_->responseBytes[4],
        impl_->responseBytes[3],
        impl_->responseBytes[2],
        impl_->responseBytes[1]
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
    const unsigned char unitByte = impl_->responseBytes[0];

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
