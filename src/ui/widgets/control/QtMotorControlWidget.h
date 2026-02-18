#ifndef CLEANGRADUATOR_QTMOTORCONTROLWIDGET_H
#define CLEANGRADUATOR_QTMOTORCONTROLWIDGET_H


#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "viewmodels/Observable.h"


namespace mvvm {
    class MotorControlViewModel;
}

namespace ui {

    class QtMotorControlWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit QtMotorControlWidget(
            mvvm::MotorControlViewModel& vm,
            QWidget* parent = nullptr);

        ~QtMotorControlWidget() override;

    private:
        void setupUi();
        void bind();

    private:
        mvvm::MotorControlViewModel& vm_;

        QLabel* statusLabel_{nullptr};
        QSpinBox* frequencySpin_{nullptr};
        QComboBox* directionBox_{nullptr};
        QPushButton* startButton_{nullptr};
        QPushButton* stopButton_{nullptr};

        mvvm::Observable<bool>::Subscription runningSub_;
    };

} // namespace ui


#endif //CLEANGRADUATOR_QTMOTORCONTROLWIDGET_H