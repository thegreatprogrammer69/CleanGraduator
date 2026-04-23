#ifndef CLEANGRADUATOR_QTCALIBRATIONSESSIONCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONSESSIONCONTROLWIDGET_H

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "viewmodels/Observable.h"

namespace mvvm {
class CalibrationSessionControlViewModel;
}

namespace ui {

class QtCalibrationSessionControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QtCalibrationSessionControlWidget(mvvm::CalibrationSessionControlViewModel& vm, QWidget* parent = nullptr);
    ~QtCalibrationSessionControlWidget() override;

private:
    void setupUi();
    void bind();

private:
    mvvm::CalibrationSessionControlViewModel& vm_;

    QLabel* errorLabel_{nullptr};
    QCheckBox* reverseModeCheckBox_{nullptr};
    QCheckBox* slowdownCheckBox_{nullptr};
    QCheckBox* playValveCheckBox_{nullptr};
    QCheckBox* kuCheckBox_{nullptr};
    QCheckBox* centeredMarkCheckBox_{nullptr};
    QPushButton* startButton_{nullptr};
    QPushButton* stopButton_{nullptr};
    QPushButton* aimButton_{nullptr};

    mvvm::Observable<std::string>::Subscription errorSub_;
    mvvm::Observable<bool>::Subscription kuModeSub_;
    mvvm::Observable<bool>::Subscription centeredMarkSub_;
    mvvm::Observable<bool>::Subscription reverseModeSub_;
    mvvm::Observable<bool>::Subscription slowdownSub_;
    mvvm::Observable<bool>::Subscription playValveSub_;
    mvvm::Observable<bool>::Subscription canStartSub_;
    mvvm::Observable<bool>::Subscription canStopSub_;
};

} // namespace ui

#endif // CLEANGRADUATOR_QTCALIBRATIONSESSIONCONTROLWIDGET_H
