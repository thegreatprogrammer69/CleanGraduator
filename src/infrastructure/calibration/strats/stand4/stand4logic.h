#ifndef CLEANGRADUATOR_STAND4LOGIC_H
#define CLEANGRADUATOR_STAND4LOGIC_H

#include <stdexcept>

#include "domain/core/calibration/common/PressurePoints.h"
#include "domain/core/measurement/Pressure.h"

namespace infra::calib::stand4 {
    inline void ensureHasRequiredPoints(const domain::common::PressurePoints& p, const char* fn_name)
    {
        if (p.value.empty()) {
            throw std::invalid_argument(std::string(fn_name) + ": pressure points are empty");
        }

        if (p.value.size() < 2) {
            throw std::invalid_argument(std::string(fn_name) + ": at least two pressure points are required");
        }
    }

    inline domain::common::Pressure computePreloadPressure(const domain::common::PressurePoints &p) {
        ensureHasRequiredPoints(p, "computePreloadPressure");

        const auto& last_point = p.value.back();
        const auto& first_point = p.value[1];
        if (last_point.kgfcm2() < 200) {
            return first_point * 0.87;
        }
        if (last_point.kgfcm2() < 300) {
            return first_point * 0.35;
        }
        return first_point * 0.26;
    }

    inline domain::common::Pressure computeTargetPressure(const domain::common::PressurePoints &p, const double overload = 0.075) {
        ensureHasRequiredPoints(p, "computeTargetPressure");
        return p.value.back() + (p.value[1] - p.value[0]) * overload;
    }

    inline domain::common::Pressure computeLimitPressure(const domain::common::PressurePoints &p, const double overload = 0.25) {
        ensureHasRequiredPoints(p, "computeLimitPressure");
        return p.value.back() + (p.value[1] - p.value[0]) * overload;
    }

    inline domain::common::Pressure computeNominalVelocity(const domain::common::PressurePoints &p, const double durationSec = 100.) {
        if (p.value.empty()) {
            throw std::invalid_argument("computeNominalVelocity: pressure points are empty");
        }
        return p.value.back() / durationSec;
    }
}

#endif //CLEANGRADUATOR_STAND4LOGIC_H
