#ifndef CLEANGRADUATOR_STAND4LOGIC_H
#define CLEANGRADUATOR_STAND4LOGIC_H
#include "domain/core/calibration/common/PressurePoints.h"
#include "domain/core/measurement/Pressure.h"

namespace infra::calib::stand4 {
    inline domain::common::Pressure computePreloadPressure(const domain::common::PressurePoints &p) {
        const auto& last_point = p.value.back();
        if (last_point.kgfcm2() < 200) {
            return last_point * 0.87;
        }
        if (last_point.kgfcm2() < 300) {
            return last_point * 0.35;
        }
        return last_point * 0.26;
    }

    inline domain::common::Pressure computeTargetPressure(const domain::common::PressurePoints &p, const double overload = 0.075) {
        return p.value.back() + (p.value[1] - p.value[0]) * overload;
    }

    inline domain::common::Pressure computeLimitPressure(const domain::common::PressurePoints &p, const double overload = 0.25) {
        return p.value.back() + (p.value[1] - p.value[0]) * overload;
    }

    inline domain::common::Pressure computeNominalVelocity(const domain::common::PressurePoints &p, const double durationSec = 100.) {
        return p.value.back() * durationSec;
    }
}

#endif //CLEANGRADUATOR_STAND4LOGIC_H