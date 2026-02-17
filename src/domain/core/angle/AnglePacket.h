#ifndef CLEANGRADUATOR_ANGLEPACKET_H
#define CLEANGRADUATOR_ANGLEPACKET_H
#include "AngleSourceId.h"
#include "domain/core/measurement/Angle.h"
#include "domain/core/measurement/Timestamp.h"

namespace domain::common {
    struct AnglePacket {
        AngleSourceId source_id;
        Timestamp timestamp;
        Angle angle;
    };
}

#endif //CLEANGRADUATOR_ANGLEPACKET_H
