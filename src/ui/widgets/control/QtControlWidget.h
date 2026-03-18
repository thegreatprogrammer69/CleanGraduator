#ifndef CLEANGRADUATOR_QTCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCONTROLWIDGET_H

#include <QWidget>

#include "QtCalibrationSessionControlWidget.h"
#include "QtDualValveControlWidget.h"
#include "QtMotorControlWidget.h"

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

        QWidget* makeResultSection();
        QWidget* makeCalibrationSection();
        QWidget* makeControlSection();
        QWidget* makeValvesPage();
        QWidget* makeMotorPage();

    private:
        mvvm::ControlViewModel& vm_;
    };

} // namespace ui

#endif
