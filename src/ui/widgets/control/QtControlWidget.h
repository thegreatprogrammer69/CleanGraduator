#ifndef CLEANGRADUATOR_QTCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCONTROLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>

#include "QtDualValveControlWidget.h"
#include "QtMotorControlWidget.h"
#include "QtCalibrationSessionControlWidget.h"

namespace mvvm {
    class ControlViewModel;
}

namespace ui {

    class QtControlWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QtControlWidget(
            mvvm::ControlViewModel& vm,
            QWidget* parent = nullptr);

    private:
        void setupUi();

        QWidget* makeValvesPage();
        QWidget* makeMotorPage();
        QWidget* makeCalibrationPage();

    private:
        mvvm::ControlViewModel& vm_;
        QTabWidget* tabs_{nullptr};
    };

} // namespace ui

#endif