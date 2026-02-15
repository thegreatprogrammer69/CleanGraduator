#include "QtMainWindow.h"

#include "ui/widgets/layout/QtStackPanelWidget.h"
#include "ui/widgets/settings/QtSettingsWidget.h"

#include <QWidget>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QGridLayout>
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

#include "logging/QtLogViewerWidget.h"
#include "video/QtVideoSourceGridWidget.h"
#include "viewmodels/MainWindowViewModel.h"


ui::QtMainWindow::QtMainWindow(
    mvvm::MainWindowViewModel& model,
    QWidget* parent
)
    : QMainWindow(parent)
    , model_(model)
{
    // Dock с логами
    createLogDock(model_.logViewerViewModel());

    // Меню View → Logs
    auto* viewMenu = menuBar()->addMenu(tr("Диагностика"));
    viewMenu->addAction(log_dock_->toggleViewAction());


    /* ================= Central ================= */
    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    /* ================= Левая часть: камеры ================= */
    m_cameras =
        new widgets::QtVideoSourceGridWidget(
            model_.videoSourceGridViewModel(),
            this
        );

    /* ================= Правая часть ================= */
    auto* rightPanel = new QWidget(central);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    /* ================= StackPanel ================= */
    auto* panel = new ui::widgets::QtStackPanelWidget(rightPanel);

    /* =================================================
     * ================= PAGE: ПРОЦЕСС ==================
     * ================================================= */
    auto* processPage = new QWidget();
    auto* processLayout = new QVBoxLayout(processPage);
    processLayout->setContentsMargins(0, 0, 0, 0);
    processLayout->setSpacing(10);

    /* ----- Карточка: таблица ----- */
    auto* tableCard = new QFrame(processPage);
    tableCard->setObjectName("contentCard");
    tableCard->setAttribute(Qt::WA_StyledBackground, true);
    tableCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(12, 12, 12, 12);

    auto* table = new QTableWidget(8, 16, tableCard);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableLayout->addWidget(table);

    /* ----- Карточка: управление ----- */
    auto* controlCard = new QFrame(processPage);
    controlCard->setObjectName("contentCard");
    controlCard->setAttribute(Qt::WA_StyledBackground, true);
    controlCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* controlLayout = new QGridLayout(controlCard);
    controlLayout->setContentsMargins(12, 12, 12, 12);
    controlLayout->setSpacing(12);

    auto* engineDir = new QGroupBox(tr("Направление двигателя"));
    auto* dirLayout = new QHBoxLayout(engineDir);
    dirLayout->addWidget(new QRadioButton(tr("Вперёд")));
    dirLayout->addWidget(new QRadioButton(tr("Назад")));

    auto* engineCtrl = new QGroupBox(tr("Управление двигателем"));
    auto* engLayout = new QVBoxLayout(engineCtrl);
    engLayout->addWidget(new QPushButton(tr("Старт")));
    engLayout->addWidget(new QPushButton(tr("Стоп")));

    auto* valveCtrl = new QGroupBox(tr("Управление клапанами"));
    auto* valveLayout = new QVBoxLayout(valveCtrl);
    valveLayout->addWidget(new QPushButton(tr("Открыть впускной")));
    valveLayout->addWidget(new QPushButton(tr("Открыть выпускной")));
    valveLayout->addWidget(new QPushButton(tr("Закрыть оба")));

    controlLayout->addWidget(engineDir,  0, 0);
    controlLayout->addWidget(engineCtrl, 1, 0);
    controlLayout->addWidget(valveCtrl,  0, 1, 2, 1);

    processLayout->addWidget(tableCard, 1);
    processLayout->addWidget(controlCard, 0);

    /* =================================================
     * ================= PAGE: НАСТРОЙКИ ================
     * ================================================= */
    auto* settingsPage = new QWidget();
    auto* settingsLayout = new QVBoxLayout(settingsPage);
    settingsLayout->setContentsMargins(0, 0, 0, 0);
    settingsLayout->setSpacing(10);

    /* ----- Карточка: настройки ----- */
    auto* settingsCard = new QFrame(settingsPage);
    settingsCard->setObjectName("contentCard");
    settingsCard->setAttribute(Qt::WA_StyledBackground, true);
    settingsCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* settingsCardLayout = new QVBoxLayout(settingsCard);
    settingsCardLayout->setContentsMargins(12, 12, 12, 12);
    settingsCardLayout->setSpacing(8);


    auto* settingsWidget =
        new QtSettingsWidget(model_.settingsViewModel(), settingsCard);

    connect(
        settingsWidget,
        &QtSettingsWidget::crosshairAppearanceRequested,
        this,
        [] {
            // TODO: открыть диалог внешнего вида перекрестия
        }
    );

    settingsCardLayout->addWidget(settingsWidget);
    settingsLayout->addWidget(settingsCard);
    settingsLayout->addStretch(1);

    /* ================= Tabs ================= */
    panel->addTab(processPage,  tr("Процесс"));
    panel->addTab(settingsPage, tr("Настройки"));

    rightLayout->addWidget(panel, 1);

    /* ================= Root ================= */
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    root->addWidget(m_cameras);
    root->addWidget(rightPanel, 1);
}

void ui::QtMainWindow::createLogDock(mvvm::LogViewerViewModel& log_vm)
{
    log_widget_ = new QtLogViewerWidget(log_vm);

    log_dock_ = new QDockWidget(tr("Просмотрщик логов"), this);
    log_dock_->setWidget(log_widget_);

    log_dock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, log_dock_);

    log_dock_->setFloating(true);
    log_dock_->hide();
}