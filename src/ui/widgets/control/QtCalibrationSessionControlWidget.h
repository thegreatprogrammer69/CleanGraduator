#ifndef CLEANGRADUATOR_QTCALIBRATIONSESSIONCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONSESSIONCONTROLWIDGET_H

#include <QComboBox>
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
    explicit QtCalibrationSessionControlWidget(
        mvvm::CalibrationSessionControlViewModel& vm,
        QWidget* parent = nullptr);

    ~QtCalibrationSessionControlWidget() override;

private:
    void setupUi();
    void bind();

private:
    mvvm::CalibrationSessionControlViewModel& vm_;

    QLabel* errorLabel_{nullptr};
    QComboBox* modeCombo_{nullptr};

    QPushButton* startButton_{nullptr};
    QPushButton* stopButton_{nullptr};
    QPushButton* emergencyStopButton_{nullptr};

    mvvm::Observable<std::string>::Subscription errorSub_;
    mvvm::Observable<bool>::Subscription runningSub_;
};

} // namespace ui

#endif //CLEANGRADUATOR_QTCALIBRATIONSESSIONCONTROLWIDGET_H
