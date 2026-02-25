#ifndef CLEANGRADUATOR_IVIDEOSINK_H
#define CLEANGRADUATOR_IVIDEOSINK_H
#include "domain/core/video/VideoFramePacket.h"

namespace domain::ports {

    struct IVideoSink {
        virtual ~IVideoSink() = default;

        /// Вызывается источником при поступлении нового кадра.
        /// Контракт: реализация должна быть non-blocking или минимально блокирующей.
        virtual void onVideoFrame(const common::VideoFramePacket& packet) = 0;
    };

}

#endif //CLEANGRADUATOR_IVIDEOSINK_H