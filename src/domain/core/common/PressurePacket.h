#ifndef CLEANGRADUATOR_PRESSUREPACKET_H
#define CLEANGRADUATOR_PRESSUREPACKET_H

#include "domain/core/measurement/Pressure.h"
#include "domain/core/measurement/Timestamp.h"

namespace domain::common {

    struct PressurePacket {
        Timestamp timestamp;
        Pressure pressure;
    };

}

#endif //CLEANGRADUATOR_PRESSUREPACKET_H