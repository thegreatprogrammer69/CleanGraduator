#ifndef UNTITLED_IPRESSURESENSOR_H
#define UNTITLED_IPRESSURESENSOR_H

namespace domain::ports {
    struct IPressureSourceObserver;
    struct IPressureSink;

    struct IPressureSource {
        virtual ~IPressureSource() = default;

        virtual bool start() = 0;
        virtual void stop() = 0;
        virtual bool isRunning() const noexcept = 0;

        virtual void addObserver(IPressureSourceObserver&) = 0;
        virtual void removeObserver(IPressureSourceObserver&) = 0;

        virtual void addSink(IPressureSink&) = 0;
        virtual void removeSink(IPressureSink&) = 0;
    };
}

#endif //UNTITLED_IPRESSURESENSOR_H