#ifndef CLEANGRADUATOR_REGISTRIES_H
#define CLEANGRADUATOR_REGISTRIES_H
#include <memory>
#include "Registry.h"
#include "domain/ports/inbound/IG540Stepper.h"
#include "domain/ports/inbound/IVideoStream.h"
#include "infrastructure/video/VideoStreamPorts.h"

namespace registries {
    std::shared_ptr<Registry<IVideoStream, VideoStreamPorts>> makeVideoStreamRegistry();
    std::shared_ptr<Registry<IG540Stepper, VideoStreamPorts>> makeVideoStreamRegistry();
}

#endif //CLEANGRADUATOR_REGISTRIES_H