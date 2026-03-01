#ifndef CLEANGRADUATOR_ICALIBRATIONSTRATEGY_H
#define CLEANGRADUATOR_ICALIBRATIONSTRATEGY_H
#include "domain/core/calibration/strategy/CalibrationStrategyVerdict.h"

namespace domain::common {
    struct CalibrationStrategyBeginContext;
    struct CalibrationStrategyFeedContext;
}

namespace domain::ports {
    struct IMotorDriver;
    struct ICalibrationRecorder;

    class ICalibrationStrategy {
    public:
        virtual ~ICalibrationStrategy() = default;

        virtual void bind(IMotorDriver& motor, ICalibrationRecorder& recorder) = 0;

        virtual common::CalibrationStrategyVerdict begin(const common::CalibrationStrategyBeginContext&) = 0;

        virtual common::CalibrationStrategyVerdict feed(const common::CalibrationStrategyFeedContext&) = 0;

        virtual void end() = 0;

        virtual bool isRunning() const = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONSTRATEGY_H