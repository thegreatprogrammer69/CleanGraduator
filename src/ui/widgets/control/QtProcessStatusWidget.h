#ifndef CLEANGRADUATOR_QTPROCESSSTATUSWIDGET_H
#define CLEANGRADUATOR_QTPROCESSSTATUSWIDGET_H

#include <QWidget>
#include <QTimer>

#include "viewmodels/Observable.h"
#include "domain/core/measurement/Timestamp.h"

class QLabel;

namespace mvvm {
class CalibrationSessionControlViewModel;
class AppStatusBarViewModel;
class PressureSensorStatusBarViewModel;
}

namespace ui {

class QtProcessStatusWidget final : public QWidget {
    Q_OBJECT
public:
    explicit QtProcessStatusWidget(
        mvvm::CalibrationSessionControlViewModel& calibration_vm,
        mvvm::AppStatusBarViewModel& app_status_vm,
        mvvm::PressureSensorStatusBarViewModel& pressure_vm,
        QWidget* parent = nullptr);
    ~QtProcessStatusWidget() override;

private:
    static QString formatMmSs(domain::common::Timestamp ts);
    void refresh();

    mvvm::CalibrationSessionControlViewModel& calibration_vm_;
    mvvm::AppStatusBarViewModel& app_status_vm_;
    mvvm::PressureSensorStatusBarViewModel& pressure_vm_;

    QLabel* statusLabel_{nullptr};
    QLabel* timeLabel_{nullptr};
    QLabel* pressureLabel_{nullptr};
    QLabel* speedLabel_{nullptr};

    QTimer timer_;
    mvvm::Observable<std::string>::Subscription status_sub_;
};

}

#endif
