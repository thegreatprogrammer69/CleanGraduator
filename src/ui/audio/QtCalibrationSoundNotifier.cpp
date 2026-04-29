#include "QtCalibrationSoundNotifier.h"

#include <QUrl>

namespace ui {

QtCalibrationSoundNotifier::QtCalibrationSoundNotifier(QObject* parent)
    : QObject(parent)
{
    configureEffect(forward_finished_, QStringLiteral("qrc:/audio/forward_movement_finished.wav"));
    configureEffect(backward_finished_, QStringLiteral("qrc:/audio/backward_movement_finished.wav"));
    configureEffect(process_error_, QStringLiteral("qrc:/audio/error_during_process.wav"));
}

void QtCalibrationSoundNotifier::onForwardMovementFinished()
{
    forward_finished_.play();
}

void QtCalibrationSoundNotifier::onBackwardMovementFinished()
{
    backward_finished_.play();
}

void QtCalibrationSoundNotifier::onProcessError()
{
    process_error_.play();
}

void QtCalibrationSoundNotifier::configureEffect(QSoundEffect& effect, const QString& resource_path)
{
    effect.setSource(QUrl(resource_path));
    effect.setLoopCount(1);
    effect.setVolume(1.0f);
}

} // namespace ui
