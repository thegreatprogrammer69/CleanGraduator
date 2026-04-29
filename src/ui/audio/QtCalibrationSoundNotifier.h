#ifndef CLEANGRADUATOR_QTCALIBRATIONSOUNDNOTIFIER_H
#define CLEANGRADUATOR_QTCALIBRATIONSOUNDNOTIFIER_H

#include <QObject>
#include <QSoundEffect>

#include "viewmodels/control/ICalibrationSoundNotifier.h"

namespace ui {

class QtCalibrationSoundNotifier final : public QObject, public mvvm::ICalibrationSoundNotifier {
    Q_OBJECT
public:
    explicit QtCalibrationSoundNotifier(QObject* parent = nullptr);

    void onForwardMovementFinished() override;
    void onBackwardMovementFinished() override;
    void onProcessError() override;

private:
    static void configureEffect(QSoundEffect& effect, const QString& resource_path);

    QSoundEffect forward_finished_;
    QSoundEffect backward_finished_;
    QSoundEffect process_error_;
};

} // namespace ui

#endif // CLEANGRADUATOR_QTCALIBRATIONSOUNDNOTIFIER_H
