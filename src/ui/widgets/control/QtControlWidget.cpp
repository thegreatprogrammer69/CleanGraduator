#include "QtControlWidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "viewmodels/control/ControlViewModel.h"
#include "viewmodels/control/DualValveControlViewModel.h"
#include "viewmodels/control/MotorControlViewModel.h"
#include "../calibration/result/QtCalibrationResultSaveWidget.h"
#include "QtCalibrationSessionControlWidget.h"

namespace {
QWidget* makeSectionContainer(QWidget* parent, const QString& title)
{
    auto* section = new QFrame(parent);
    section->setObjectName("contentCard");
    section->setAttribute(Qt::WA_StyledBackground, true);
    section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* layout = new QVBoxLayout(section);
    layout->setContentsMargins(8, 8, 8, 8);
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
    root->setSpacing(8);
    root->setContentsMargins(0, 0, 0, 0);

    root->addWidget(makeCalibrationSection(), 1);
    root->addWidget(makeResultSection(), 1);
    root->addWidget(makeManualSection(), 1);
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

    auto* widget = new QtCalibrationSessionControlWidget(vm_.calibrationViewModel(), section);
    layout->addWidget(widget);
    layout->addStretch();

    return section;
}

QWidget* ui::QtControlWidget::makeManualSection()
{
    auto* section = makeSectionContainer(this, tr("Ручное управление"));
    auto* layout = qobject_cast<QVBoxLayout*>(section->layout());

    auto& motor_vm = vm_.motorViewModel();
    auto& valve_vm = vm_.valvesViewModel();

    auto* grid = new QGridLayout();
    auto* forward = new QPushButton(tr("Дв. вперёд"), section);
    auto* stop = new QPushButton(tr("Стоп двигатель"), section);
    auto* backward = new QPushButton(tr("Дв. назад"), section);
    auto* close = new QPushButton(tr("Закрыть клапана"), section);
    auto* openIn = new QPushButton(tr("Открыть впускной"), section);
    auto* openOut = new QPushButton(tr("Открыть выпускной"), section);

    grid->addWidget(forward, 0, 0);
    grid->addWidget(stop, 0, 1);
    grid->addWidget(backward, 0, 2);
    grid->addWidget(close, 1, 0, 1, 3);
    grid->addWidget(openIn, 2, 0, 1, 2);
    grid->addWidget(openOut, 2, 2, 1, 1);

    constexpr int kManualMotorFrequency = 2000;
    connect(forward, &QPushButton::clicked, section, [&motor_vm] {
        motor_vm.setFrequency(kManualMotorFrequency);
        motor_vm.setDirection(domain::common::MotorDirection::Forward);
        motor_vm.start();
    });
    connect(backward, &QPushButton::clicked, section, [&motor_vm] {
        motor_vm.setFrequency(kManualMotorFrequency);
        motor_vm.setDirection(domain::common::MotorDirection::Backward);
        motor_vm.start();
    });
    connect(stop, &QPushButton::clicked, section, [&motor_vm] { motor_vm.stop(); });

    connect(close, &QPushButton::clicked, section, [&valve_vm] { valve_vm.closeFlaps(); });
    connect(openIn, &QPushButton::clicked, section, [&valve_vm] { valve_vm.openInputFlap(); });
    connect(openOut, &QPushButton::clicked, section, [&valve_vm] { valve_vm.openOutputFlap(); });

    layout->addLayout(grid);
    layout->addStretch();
    return section;
}
