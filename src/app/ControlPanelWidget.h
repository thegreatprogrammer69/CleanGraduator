#ifndef CONTROLPANELWIDGET_H
#define CONTROLPANELWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>

class ControlPanelWidget : public QWidget
{
public:
    explicit ControlPanelWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* root = new QVBoxLayout(this);
        root->setSpacing(8);

        // =============================
        // Панель переключения (как ссылки)
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

        // Группа кнопок
        auto* group = new QButtonGroup(this);
        group->setExclusive(true);
        group->addButton(btnValves, 0);
        group->addButton(btnMotor, 1);

        btnValves->setChecked(true);

        connect(group, &QButtonGroup::idClicked,
                stack_, &QStackedWidget::setCurrentIndex);

        applyStyle();
    }

private:
    QStackedWidget* stack_{};

    QPushButton* makeSwitchButton(const QString& text)
    {
        auto* btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("role", "switchLink");
        return btn;
    }

    QWidget* makeValvesPage()
    {
        auto* page = new QWidget;
        page->setAttribute(Qt::WA_StyledBackground, true);
        page->setProperty("role", "card");

        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        layout->addWidget(makeActionButton("Открыть впускной"));
        layout->addWidget(makeActionButton("Открыть выпускной"));
        layout->addWidget(makeActionButton("Закрыть оба"));
        layout->addStretch();

        return page;
    }

    QWidget* makeMotorPage()
    {
        auto* page = new QWidget;
        page->setAttribute(Qt::WA_StyledBackground, true);
        page->setProperty("role", "card");

        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(8);

        layout->addWidget(makeActionButton("Старт"));
        layout->addWidget(makeActionButton("Стоп"));
        layout->addStretch();

        return page;
    }

    QPushButton* makeActionButton(const QString& text)
    {
        auto* btn = new QPushButton(text);
        btn->setMinimumHeight(40);
        return btn;
    }

    void applyStyle()
    {
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
           Card (как GroupBox без title)
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
};

#endif
