#include "QtMainWindow.h"

#include "ui/widgets/layout/QtStackPanelWidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QLabel>
#include <QStyle>
#include <QVariant>
#include <QGroupBox>
#include <QRadioButton>
#include <QTableWidget>
#include <QHeaderView>

#include "video/QtVideoSourceGridWidget.h"
#include "viewmodels/MainWindowViewModel.h"


static QWidget* makeProcessPage(QWidget *parent = nullptr)
{
    auto *w = new QWidget(parent);
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(14, 14, 14, 14);
    l->setSpacing(10);

    auto *title = new QLabel(QStringLiteral("Процесс"), w);
    title->setObjectName("pageTitle");

    auto *hint = new QLabel(QStringLiteral("Здесь можно показывать статус, логи, прогресс, управление запуском и т.д."), w);
    hint->setObjectName("hintText");
    hint->setWordWrap(true);

    l->addWidget(title);
    l->addWidget(hint);
    l->addStretch(1);
    return w;
}

static QWidget* makeSettingsPage(QWidget *parent = nullptr)
{
    auto *w = new QWidget(parent);
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(14, 14, 14, 14);
    l->setSpacing(10);

    auto *title = new QLabel(QStringLiteral("Настройки"), w);
    title->setObjectName("pageTitle");

    auto *hint = new QLabel(QStringLiteral("Здесь — параметры камер, сеть, запись, раскладка и т.п."), w);
    hint->setObjectName("hintText");
    hint->setWordWrap(true);

    l->addWidget(title);
    l->addWidget(hint);
    l->addStretch(1);
    return w;
}

static QPushButton* makeSegmentButton(const QString &text, QWidget *parent = nullptr)
{
    auto *b = new QPushButton(text, parent);
    b->setObjectName("segmentBtn");
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

ui::QtMainWindow::QtMainWindow(mvvm::MainWindowViewModel& model, QWidget *parent)
    : QMainWindow(parent), model_(model)
{
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    /* ================= Левая часть: камеры ================= */
    const double aspectWH = 4.0 / 3.0;
    m_cameras = new widgets::QtVideoSourceGridWidget(model.videoSourceGridModel(), this);

    /* ================= Правая часть ================= */
    auto *rightPanel = new QWidget(central);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    /* ========= StackPanelWidget (табы + стек) ========= */
    auto *panel = new ui::widgets::QtStackPanelWidget(rightPanel);

    /* =================================================
     * =============== PAGE: ПРОЦЕСС ====================
     * ================================================= */
    auto *processPage = new QWidget();
    auto *processLayout = new QVBoxLayout(processPage);
    processLayout->setContentsMargins(0, 0, 0, 0);
    processLayout->setSpacing(10);

    /* ----- Карточка 1: таблица ----- */
    auto *tableCard = new QFrame(processPage);
    tableCard->setObjectName("contentCard");
    tableCard->setAttribute(Qt::WA_StyledBackground, true);
    tableCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(12, 12, 12, 12);

    auto *table = new QTableWidget(8, 16, tableCard);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableLayout->addWidget(table);

    /* ----- Карточка 2: панель управления ----- */
    auto *controlCard = new QFrame(processPage);
    controlCard->setObjectName("contentCard");
    controlCard->setAttribute(Qt::WA_StyledBackground, true);
    controlCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *controlLayout = new QGridLayout(controlCard);
    controlLayout->setContentsMargins(12, 12, 12, 12);
    controlLayout->setSpacing(12);

    /* Направление двигателя */
    auto *engineDir = new QGroupBox("Направление двигателя");
    auto *dirLayout = new QHBoxLayout(engineDir);
    dirLayout->addWidget(new QRadioButton("Вперёд"));
    dirLayout->addWidget(new QRadioButton("Назад"));

    /* Управление двигателем */
    auto *engineCtrl = new QGroupBox("Управление двигателем");
    auto *engLayout = new QVBoxLayout(engineCtrl);
    engLayout->addWidget(new QPushButton("Старт"));
    engLayout->addWidget(new QPushButton("Стоп"));

    /* Управление клапанами */
    auto *valveCtrl = new QGroupBox("Управление клапанами");
    auto *valveLayout = new QVBoxLayout(valveCtrl);
    valveLayout->addWidget(new QPushButton("Открыть впускной"));
    valveLayout->addWidget(new QPushButton("Открыть выпускной"));
    valveLayout->addWidget(new QPushButton("Закрыть оба"));

    controlLayout->addWidget(engineDir,  0, 0);
    controlLayout->addWidget(engineCtrl, 1, 0);
    controlLayout->addWidget(valveCtrl,  0, 1, 2, 1);

    processLayout->addWidget(tableCard, 1);
    processLayout->addWidget(controlCard, 0);

    /* ================= PAGE: НАСТРОЙКИ ================= */
    auto *settingsPage = new QWidget();
    // позже можно наполнить

    /* ========= Добавляем страницы как табы ========= */
    panel->addTab(processPage,  "Процесс");
    panel->addTab(settingsPage, "Настройки");

    /* ================= Сборка ================= */
    rightLayout->addWidget(panel, 1);

    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    root->addWidget(m_cameras);
    root->addWidget(rightPanel, 1);
}
