#ifndef CLEANGRADUATOR_ICALIBRATIONSTRATEGY_H
#define CLEANGRADUATOR_ICALIBRATIONSTRATEGY_H

namespace domain::common {
    struct CalibrationBeginContext;
    struct CalibrationFeedContext;
}

namespace domain::ports {
    struct IValveDriver;
    struct IMotorDriver;
    struct ICalibrationRecorder;

    enum class CalibrationDecisionType {
        None,
        SwitchToBackward,
        Finish,
        Fault
    };

    class ICalibrationStrategy {
    public:
        virtual ~ICalibrationStrategy() = default;

        virtual void bind(IMotorDriver& motor, IValveDriver& valve, ICalibrationRecorder& recorder) = 0;

        virtual void begin(const common::CalibrationBeginContext&) = 0;

        virtual CalibrationDecisionType feed(const common::CalibrationFeedContext&) = 0;

        virtual void end() = 0;

        virtual bool isRunning() const = 0;
    };
}

#endif //CLEANGRADUATOR_ICALIBRATIONSTRATEGY_H