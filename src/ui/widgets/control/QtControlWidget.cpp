//
// Created by mint on 18.02.2026.
//

#include "QtControlWidget.h"

#include "viewmodels/control/ControlViewModel.h"

ui::QtControlWidget::QtControlWidget(mvvm::ControlViewModel &vm, QWidget *parent): QWidget(parent)
                                                                                   , vm_(vm) {
    setupUi();
    applyStyle();
}

void ui::QtControlWidget::setupUi() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(8);

    // =============================
    // Панель переключения
    // =============================
    auto* switchBar = new QHBoxLayout;
    switchBar->setSpacing(16);

    auto* btnValves = makeSwitchButton("Клапаны");
    auto* btnMotor  = makeSwitchButton("Управление двигателем");

    switchBar->addWidget(btnValves);
    switchBar->addWidget(btnMotor);
    switchBar->addStretch();

    root->addLayout(switchBar);

    // =============================
    // Stack
    // =============================
    stack_ = new QStackedWidget;
    root->addWidget(stack_);

    stack_->addWidget(makeValvesPage());
    stack_->addWidget(makeMotorPage());

    auto* group = new QButtonGroup(this);
    group->setExclusive(true);
    group->addButton(btnValves, 0);
    group->addButton(btnMotor, 1);

    btnValves->setChecked(true);

    connect(group,
            QOverload<int>::of(&QButtonGroup::buttonClicked),
            stack_,
            &QStackedWidget::setCurrentIndex);
}

QPushButton * ui::QtControlWidget::makeSwitchButton(const QString &text) {
    auto* btn = new QPushButton(text);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("role", "switchLink");
    return btn;
}

QWidget * ui::QtControlWidget::makeValvesPage() {
    auto* page = new QWidget;
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setProperty("role", "card");

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto* valvesWidget =
            new QtDualValveControlWidget(vm_.valvesViewModel(), page);

    layout->addWidget(valvesWidget);
    layout->addStretch();

    return page;
}

QWidget * ui::QtControlWidget::makeMotorPage() {
    auto* page = new QWidget;
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setProperty("role", "card");

    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto* motorWidget =
            new QtMotorControlWidget(vm_.motorViewModel(), page);

    layout->addWidget(motorWidget);
    layout->addStretch();

    return page;
}

void ui::QtControlWidget::applyStyle() {
    setStyleSheet(R"(

        /* =========================
           Switch Links
        ==========================*/
        QPushButton[role="switchLink"] {
            background: transparent;
            border: none;
            padding: 4px 0;
            font-size: 15px;
            font-weight: 600;
            color: #374151;
        }

        QPushButton[role="switchLink"]:hover {
            color: #2563EB;
            text-decoration: underline;
        }

        QPushButton[role="switchLink"]:checked {
            color: #111827;
            border-bottom: 2px solid #2563EB;
        }

        /* =========================
           Card
        ==========================*/
        QWidget[role="card"] {
            background: #FFFFFF;
            border: 1px solid #9CA3AF;
            border-radius: 6px;
        }

        /* =========================
           Action Buttons
        ==========================*/
        QPushButton {
            background: #F9FAFB;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 8px;
            font-size: 14px;
        }

        QPushButton:hover {
            background: #EFF6FF;
            border: 1px solid #3B82F6;
        }

        QPushButton:pressed {
            background: #DBEAFE;
        }

        )");
}
