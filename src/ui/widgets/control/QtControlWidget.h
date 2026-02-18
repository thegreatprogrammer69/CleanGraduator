#ifndef CLEANGRADUATOR_QTCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCONTROLWIDGET_H


#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

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

    QPushButton* makeSwitchButton(const QString& text);

    QWidget* makeValvesPage();

    QWidget* makeMotorPage();

    void applyStyle();

private:
    mvvm::ControlViewModel& vm_;
    QStackedWidget* stack_{nullptr};
};

} // namespace ui


#endif //CLEANGRADUATOR_QTCONTROLWIDGET_H