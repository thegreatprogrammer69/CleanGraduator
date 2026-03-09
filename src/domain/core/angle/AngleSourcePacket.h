#ifndef CLEANGRADUATOR_ANGLEPACKET_H
#define CLEANGRADUATOR_ANGLEPACKET_H
#include "SourceId.h"
#include "domain/core/measurement/Angle.h"
#include "domain/core/measurement/Timestamp.h"

namespace domain::common {
    struct AngleSourcePacket {
        SourceId source_id;
        Timestamp timestamp;
        Angle angle;
    };
}

#endif //CLEANGRADUATOR_ANGLEPACKET_H
