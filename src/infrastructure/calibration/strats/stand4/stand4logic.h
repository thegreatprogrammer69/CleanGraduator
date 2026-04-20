#ifndef CLEANGRADUATOR_STAND4LOGIC_H
#define CLEANGRADUATOR_STAND4LOGIC_H
#include "domain/core/calibration/common/PressurePoints.h"
#include "domain/core/measurement/Pressure.h"

namespace infra::calib::stand4 {
    inline bool hasValidPointsForStand4(const domain::common::PressurePoints& p) {
        return p.value.size() >= 2;
    }

    inline domain::common::Pressure nominalStep(const domain::common::PressurePoints& p) {
        if (!hasValidPointsForStand4(p)) {
            return domain::common::Pressure(0.0f, domain::common::PressureUnit::kgf_cm2);
        }

        return p.value[1] - p.value[0];
    }

    inline domain::common::Pressure computePreloadPressure(const domain::common::PressurePoints &p) {
        if (!hasValidPointsForStand4(p)) {
            return domain::common::Pressure(0.0f, domain::common::PressureUnit::kgf_cm2);
        }

        const auto& last_point = p.value.back();
        const auto& first_step_point = p.value[1];
        if (last_point.kgfcm2() < 200) {
            return first_step_point * 0.87;
        }
        if (last_point.kgfcm2() < 300) {
            return first_step_point * 0.35;
        }
        return first_step_point * 0.26;
    }

    inline domain::common::Pressure computeTargetPressure(const domain::common::PressurePoints &p, const double overload = 0.075) {
        if (!hasValidPointsForStand4(p)) {
            return domain::common::Pressure(0.0f, domain::common::PressureUnit::kgf_cm2);
        }

        return p.value.back() + nominalStep(p) * overload;
    }

    inline domain::common::Pressure computeLimitPressure(const domain::common::PressurePoints &p, const double overload = 0.25) {
        if (!hasValidPointsForStand4(p)) {
            return domain::common::Pressure(0.0f, domain::common::PressureUnit::kgf_cm2);
        }

        return p.value.back() + nominalStep(p) * overload;
    }

    inline domain::common::Pressure computeNominalVelocity(const domain::common::PressurePoints &p, const double durationSec = 100.) {
        if (p.value.empty() || durationSec <= 0.0) {
            return domain::common::Pressure(0.0f, domain::common::PressureUnit::kgf_cm2);
        }

        return p.value.back() / durationSec;
    }
}

#endif //CLEANGRADUATOR_STAND4LOGIC_H
