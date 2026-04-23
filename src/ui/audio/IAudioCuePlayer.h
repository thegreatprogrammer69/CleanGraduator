#ifndef CLEANGRADUATOR_IAUDIOCUEPLAYER_H
#define CLEANGRADUATOR_IAUDIOCUEPLAYER_H

#include "domain/core/calibration/common/CalibrationAudioCue.h"

namespace ui {

class IAudioCuePlayer {
public:
    virtual ~IAudioCuePlayer() = default;
    virtual void play(domain::common::CalibrationAudioCue cue) = 0;
};

} // namespace ui

#endif // CLEANGRADUATOR_IAUDIOCUEPLAYER_H
