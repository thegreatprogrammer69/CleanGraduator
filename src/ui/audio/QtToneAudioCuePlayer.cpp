#include "QtToneAudioCuePlayer.h"

#include <cmath>

#include <QtMath>

namespace {
constexpr int kSampleRateHz = 48000;
constexpr double kAmplitude = 0.30;
constexpr double kTwoPi = 6.28318530717958647692;
}

ui::QtToneAudioCuePlayer::QtToneAudioCuePlayer(QObject* parent)
    : QObject(parent)
{
    format_.setSampleRate(kSampleRateHz);
    format_.setChannelCount(1);
    format_.setSampleSize(16);
    format_.setCodec("audio/pcm");
    format_.setByteOrder(QAudioFormat::LittleEndian);
    format_.setSampleType(QAudioFormat::SignedInt);

    output_ = new QAudioOutput(format_, this);
    output_->setVolume(1.0);
    buffer_.open(QIODevice::ReadOnly);
}

ui::QtToneAudioCuePlayer::~QtToneAudioCuePlayer()
{
    if (output_ != nullptr) {
        output_->stop();
    }
    buffer_.close();
}

void ui::QtToneAudioCuePlayer::play(const domain::common::CalibrationAudioCue cue)
{
    pcm_data_ = buildPcm16Mono(sequenceFor(cue));
    buffer_.setData(pcm_data_);
    buffer_.seek(0);

    output_->stop();
    output_->start(&buffer_);
}

QByteArray ui::QtToneAudioCuePlayer::buildPcm16Mono(const std::vector<Step>& steps) const
{
    QByteArray pcm;

    for (const auto& step : steps) {
        const int sample_count = qMax(1, qRound(step.duration_sec * static_cast<double>(kSampleRateHz)));
        const int start_size = pcm.size();
        pcm.resize(start_size + sample_count * static_cast<int>(sizeof(qint16)));
        auto* out = reinterpret_cast<qint16*>(pcm.data() + start_size);

        double phase = 0.0;
        const double phase_delta = step.is_silence
            ? 0.0
            : kTwoPi * step.frequency_hz / static_cast<double>(kSampleRateHz);

        for (int i = 0; i < sample_count; ++i) {
            const double envelope = 0.5 * (1.0 - qCos(kTwoPi * static_cast<double>(i) / static_cast<double>(sample_count)));
            const double sample = step.is_silence ? 0.0 : triangleSample(phase) * envelope * kAmplitude;
            out[i] = static_cast<qint16>(qBound(-32767.0, sample * 32767.0, 32767.0));
            phase += phase_delta;
        }
    }

    return pcm;
}

std::vector<ui::QtToneAudioCuePlayer::Step> ui::QtToneAudioCuePlayer::sequenceFor(
    const domain::common::CalibrationAudioCue cue)
{
    using Cue = domain::common::CalibrationAudioCue;

    switch (cue) {
        case Cue::ProcessComplete:
            return {
                {false, 784.0, 0.10}, {true, 0.0, 0.04}, {false, 988.0, 0.10},
                {true, 0.0, 0.04}, {false, 1175.0, 0.12}, {true, 0.0, 0.05},
                {false, 1568.0, 0.24}, {true, 0.0, 0.05}, {false, 1175.0, 0.10},
                {true, 0.0, 0.03}, {false, 1568.0, 0.22}
            };
        case Cue::Critical:
            return {
                {false, 1175.0, 0.14}, {true, 0.0, 0.05}, {false, 784.0, 0.14},
                {true, 0.0, 0.05}, {false, 1175.0, 0.14}, {true, 0.0, 0.10},
                {false, 659.0, 0.22}, {true, 0.0, 0.24},
                {false, 1175.0, 0.14}, {true, 0.0, 0.05}, {false, 784.0, 0.14},
                {true, 0.0, 0.05}, {false, 1175.0, 0.14}, {true, 0.0, 0.10},
                {false, 659.0, 0.22}, {true, 0.0, 0.24}
            };
        case Cue::Alarm:
            return {
                {false, 1175.0, 0.11}, {true, 0.0, 0.04}, {false, 988.0, 0.11},
                {true, 0.0, 0.04}, {false, 784.0, 0.24}
            };
        case Cue::OperatorAction:
            return {
                {false, 988.0, 0.28}
            };
    }

    return {};
}

double ui::QtToneAudioCuePlayer::triangleSample(const double phase)
{
    const double normalized = std::fmod(phase / kTwoPi, 1.0);
    const double wrapped = normalized < 0.0 ? normalized + 1.0 : normalized;
    return 1.0 - 4.0 * std::abs(wrapped - 0.5);
}
