#ifndef CLEANGRADUATOR_ICAMERASTREAM_H
#define CLEANGRADUATOR_ICAMERASTREAM_H


namespace domain::ports {
    struct IVideoSourceObserver;
    struct IVideoSource {
        virtual ~IVideoSource() = default;
        virtual bool open() = 0;
        virtual void close() = 0;
        virtual void addObserver(IVideoSourceObserver&) = 0;
        virtual void removeObserver(IVideoSourceObserver&) = 0;
    };
}

#endif //CLEANGRADUATOR_ICAMERASTREAM_H