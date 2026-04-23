#ifndef CLEANGRADUATOR_QTCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCONTROLWIDGET_H

#include <QWidget>

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
        QWidget* makeManualSection();

    private:
        mvvm::ControlViewModel& vm_;
    };

} // namespace ui

#endif
