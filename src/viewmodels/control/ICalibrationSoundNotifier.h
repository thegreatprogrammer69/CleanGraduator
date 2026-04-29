#ifndef CLEANGRADUATOR_ICALIBRATIONSOUNDNOTIFIER_H
#define CLEANGRADUATOR_ICALIBRATIONSOUNDNOTIFIER_H

namespace mvvm {

struct ICalibrationSoundNotifier {
    virtual ~ICalibrationSoundNotifier() = default;

    virtual void onForwardMovementFinished() = 0;
    virtual void onBackwardMovementFinished() = 0;
    virtual void onProcessError() = 0;
};

} // namespace mvvm

#endif // CLEANGRADUATOR_ICALIBRATIONSOUNDNOTIFIER_H
