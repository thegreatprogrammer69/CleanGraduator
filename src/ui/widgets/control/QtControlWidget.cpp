#include "QtControlWidget.h"

#include <QHBoxLayout>
#include <QTabWidget>
#include <QVBoxLayout>

#include "viewmodels/control/ControlViewModel.h"

ui::QtControlWidget::QtControlWidget(
        mvvm::ControlViewModel& vm,
        QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
}

void ui::QtControlWidget::setupUi()
{
    auto* root = new QHBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(0,0,0,0);

    // -------------------------
    // Левая часть — градуировка
    // -------------------------

    auto* calibrationTabs = new QTabWidget(this);
    calibrationTabs->setTabPosition(QTabWidget::North);


    calibrationTabs->addTab(
        makeCalibrationPage(),
        tr("Градуировка"));

    root->addWidget(calibrationTabs);

    // -------------------------
    // Правая часть — управление
    // -------------------------

    tabs_ = new QTabWidget(this);
    tabs_->setTabPosition(QTabWidget::North);

    tabs_->addTab(
        makeValvesPage(),
        tr("Клапаны"));

    tabs_->addTab(
        makeMotorPage(),
        tr("Двигатель"));

    root->addWidget(tabs_, 1);
}

QWidget* ui::QtControlWidget::makeValvesPage()
{
    auto* page = new QWidget;

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16,16,16,16);

    auto* widget =
        new QtDualValveControlWidget(
            vm_.valvesViewModel(),
            page);

    layout->addWidget(widget);

    return page;
}

QWidget* ui::QtControlWidget::makeMotorPage()
{
    auto* page = new QWidget;

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16,16,16,16);

    auto* widget =
        new QtMotorControlWidget(
            vm_.motorViewModel(),
            page);

    layout->addWidget(widget);

    return page;
}

QWidget* ui::QtControlWidget::makeCalibrationPage()
{
    auto* page = new QWidget;

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16,0,16,16);

    auto* widget =
        new QtCalibrationSessionControlWidget(
            vm_.calibrationViewModel(),
            page);

    layout->addWidget(widget);
    layout->addStretch();

    return page;
}