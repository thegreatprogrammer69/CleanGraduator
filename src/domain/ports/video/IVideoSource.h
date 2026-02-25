#ifndef CLEANGRADUATOR_ICAMERASTREAM_H
#define CLEANGRADUATOR_ICAMERASTREAM_H


namespace domain::ports {
    struct IVideoSink;
    struct IVideoSourceObserver;
    struct IVideoSource {
        virtual ~IVideoSource() = default;

        virtual void open() = 0;
        virtual void close() = 0;
        virtual bool isRunning() const noexcept = 0;

        virtual void addObserver(IVideoSourceObserver&) = 0;
        virtual void removeObserver(IVideoSourceObserver&) = 0;

        virtual void addSink(IVideoSink&) = 0;
        virtual void removeSink(IVideoSink&) = 0;
    };
}

#endif //CLEANGRADUATOR_ICAMERASTREAM_H