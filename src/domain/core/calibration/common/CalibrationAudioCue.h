#ifndef CLEANGRADUATOR_CALIBRATIONAUDIOCUE_H
#define CLEANGRADUATOR_CALIBRATIONAUDIOCUE_H

namespace domain::common {

enum class CalibrationAudioCue {
    ProcessComplete,
    Critical,
    Alarm,
    OperatorAction
};

} // namespace domain::common

#endif // CLEANGRADUATOR_CALIBRATIONAUDIOCUE_H
