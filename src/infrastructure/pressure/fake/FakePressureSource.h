#ifndef CLEANGRADUATOR_FAKEPRESSURESOURCE_H
#define CLEANGRADUATOR_FAKEPRESSURESOURCE_H

#include <memory>

#include <domain/ports/pressure/IPressureSource.h>

#include "FakePressureSourceConfig.h"
#include "../PressureSourcePorts.h"
#include "infrastructure/pressure/PressureSourceNotifier.h"
#include "domain/fmt/Logger.h"
#include "infrastructure/utils/thread/ThreadWorker.h"

namespace domain::ports { class IClock; }

namespace infra::pressure {

    class FakePressureSource final : public domain::ports::IPressureSource
    {
    public:
        FakePressureSource(PressureSourcePorts ports,
                           FakePressureSourceConfig config);
        ~FakePressureSource() override;

        bool start() override;
        void stop() override;
        bool isRunning() const noexcept override;

        void addObserver(domain::ports::IPressureSourceObserver& observer) override;
        void removeObserver(domain::ports::IPressureSourceObserver& observer) override;

        void addSink(domain::ports::IPressureSink& sink) override;
        void removeSink(domain::ports::IPressureSink& sink) override;

    private:
        void loop();

    private:
        FakePressureSourceConfig config_;
        fmt::Logger logger_;
        domain::ports::IClock& clock_;
        detail::PressureSourceNotifier notifier_;

        std::unique_ptr<utils::thread::ThreadWorker> worker_;
        bool running_{false};
    };

}

#endif