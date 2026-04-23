#ifndef CLEANGRADUATOR_QTTONEAUDIOCUEPLAYER_H
#define CLEANGRADUATOR_QTTONEAUDIOCUEPLAYER_H

#include <QAudioOutput>
#include <QBuffer>
#include <QObject>
#include <QByteArray>
#include <vector>

#include "IAudioCuePlayer.h"

namespace ui {

class QtToneAudioCuePlayer final : public QObject, public IAudioCuePlayer {
    Q_OBJECT
public:
    explicit QtToneAudioCuePlayer(QObject* parent = nullptr);
    ~QtToneAudioCuePlayer() override;

    void play(domain::common::CalibrationAudioCue cue) override;

private:
    struct Step {
        bool is_silence{false};
        double frequency_hz{0.0};
        double duration_sec{0.0};
    };

    QByteArray buildPcm16Mono(const std::vector<Step>& steps) const;
    static std::vector<Step> sequenceFor(domain::common::CalibrationAudioCue cue);
    static double triangleSample(double phase);

    QAudioFormat format_;
    QAudioOutput* output_{nullptr};
    QBuffer buffer_;
    QByteArray pcm_data_;
};

} // namespace ui

#endif // CLEANGRADUATOR_QTTONEAUDIOCUEPLAYER_H
