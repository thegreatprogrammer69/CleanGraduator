#ifndef CLEANGRADUATOR_FAKEPRESSURESOURCE_H
#define CLEANGRADUATOR_FAKEPRESSURESOURCE_H

#include <atomic>
#include <thread>
#include <mutex>

#include <domain/ports/pressure/IPressureSource.h>

#include "FakePressureSourceConfig.h"
#include "../PressureSourcePorts.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "domain/fmt/Logger.h"

namespace domain::ports { class IClock; }

namespace infra::pressure {

    class FakePressureSource final : public domain::ports::IPressureSource {
    public:
        FakePressureSource(PressureSourcePorts ports,
                           FakePressureSourceConfig config);
        ~FakePressureSource() override;

        bool start() override;
        void stop() override;
        bool isRunning() const noexcept override;

        void addObserver(domain::ports::IPressureSourceObserver& observer) override;
        void removeObserver(domain::ports::IPressureSourceObserver& observer) override;

    private:
        void run();

    private:
        FakePressureSourceConfig config_;
        fmt::Logger logger_;
        domain::ports::IClock& clock_;
        detail::PressureSourceNotifier notifier_;

        std::atomic<bool> stop_requested_{true};
        std::atomic<bool> running_{false};
        std::thread worker_;
        std::mutex lifecycle_mtx_;
    };

}

#endif