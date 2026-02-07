#ifndef CLEANGRADUATOR_VIDEOFRAME_H
#define CLEANGRADUATOR_VIDEOFRAME_H

#include <memory>
#include "PixelFormat.h"
#include "VideoBuffer.h"

namespace domain::common {

struct VideoFrame {
    int width = 0;
    int height = 0;
    PixelFormat format = PixelFormat::RGB24;
    VideoBuffer buffer;
};

using VideoFramePtr = std::shared_ptr<VideoFrame>;

}

#endif //CLEANGRADUATOR_VIDEOFRAME_H