#ifndef CLEANGRADUATOR_IVIDEOSINK_H
#define CLEANGRADUATOR_IVIDEOSINK_H

namespace domain::common {
    struct VideoFramePacket;
}

namespace domain::ports {

    struct IVideoSink {
        virtual ~IVideoSink() = default;

        /// Вызывается источником при поступлении нового кадра.
        /// Контракт: реализация должна быть non-blocking или минимально блокирующей.
        virtual void onVideoFrame(const common::VideoFramePacket&) = 0;
    };

}

#endif //CLEANGRADUATOR_IVIDEOSINK_H