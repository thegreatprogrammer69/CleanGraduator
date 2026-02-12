#ifndef CLEANGRADUATOR_VIDEOOUTPUT_H
#define CLEANGRADUATOR_VIDEOOUTPUT_H
#include "VideoFrame.h"
#include "domain/core/measurement/Timestamp.h"

namespace domain::common {

    struct VideoFramePacket {
        Timestamp timestamp;
        VideoFramePtr frame;
    };

}


#endif //CLEANGRADUATOR_VIDEOOUTPUT_H