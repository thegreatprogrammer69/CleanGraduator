#ifndef CLEANGRADUATOR_APPLICATION_PRESSUREUNIT_H
#define CLEANGRADUATOR_APPLICATION_PRESSUREUNIT_H

#include "domain/core/measurement/PressureUnit.h"

namespace application::models {
    struct PressureUnit {
        domain::common::PressureUnit unit;
    };
}

#endif // CLEANGRADUATOR_APPLICATION_PRESSUREUNIT_H
