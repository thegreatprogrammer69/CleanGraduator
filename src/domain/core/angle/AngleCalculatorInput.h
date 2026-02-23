#ifndef CLEANGRADUATOR_ANGLEMETERINPUT_H
#define CLEANGRADUATOR_ANGLEMETERINPUT_H
#include "domain/core/video/VideoFrame.h"

namespace domain::common {
    struct AngleCalculatorInput {
        VideoFramePtr frame;
    };
}


#endif //CLEANGRADUATOR_ANGLEMETERINPUT_H