#include "QtControlWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

#include "viewmodels/control/ControlViewModel.h"
#include "../calibration/result/QtCalibrationResultSaveWidget.h"

namespace {
QWidget* makeSectionContainer(QWidget* parent, const QString& title)
{
    auto* section = new QFrame(parent);
    section->setObjectName("contentCard");
    section->setAttribute(Qt::WA_StyledBackground, true);
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* layout = new QVBoxLayout(section);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto* titleLabel = new QLabel(title, section);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    layout->addWidget(titleLabel);

    return section;
}
}

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
    root->setContentsMargins(0, 0, 0, 0);

    root->addWidget(makeResultSection(), 1);
    root->addWidget(makeCalibrationSection(), 1);
    root->addWidget(makeControlSection(), 1);
}

QWidget* ui::QtControlWidget::makeResultSection()
{
    auto* section = makeSectionContainer(this, tr("Результат"));
    auto* layout = qobject_cast<QVBoxLayout*>(section->layout());

    auto* widget = new QtCalibrationResultSaveWidget(vm_.calibrationResultSaveViewModel(), section);
    layout->addWidget(widget);
    layout->addStretch();

    return section;
}

QWidget* ui::QtControlWidget::makeCalibrationSection()
{
    auto* section = makeSectionContainer(this, tr("Градуировка"));
    auto* layout = qobject_cast<QVBoxLayout*>(section->layout());

    auto* widget = new QtCalibrationSessionControlWidget(
        vm_.calibrationViewModel(),
        section);

    layout->addWidget(widget);
    layout->addStretch();

    return section;
}

QWidget* ui::QtControlWidget::makeControlSection()
{
    auto* section = makeSectionContainer(this, tr("Управление мотором/клапанами"));
    auto* layout = qobject_cast<QVBoxLayout*>(section->layout());

    auto* tabs = new QTabWidget(section);
    tabs->setTabPosition(QTabWidget::North);
    tabs->addTab(makeValvesPage(), tr("Клапаны"));
    tabs->addTab(makeMotorPage(), tr("Двигатель"));

    layout->addWidget(tabs);

    return section;
}

QWidget* ui::QtControlWidget::makeValvesPage()
{
    auto* page = new QWidget;

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(4, 4, 4, 4);

    auto* widget =
        new QtDualValveControlWidget(
            vm_.valvesViewModel(),
            page);

    layout->addWidget(widget);
    layout->addStretch();

    return page;
}

QWidget* ui::QtControlWidget::makeMotorPage()
{
    auto* page = new QWidget;

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(4, 4, 4, 4);

    auto* widget =
        new QtMotorControlWidget(
            vm_.motorViewModel(),
            page);

    layout->addWidget(widget);
    layout->addStretch();

    return page;
}
