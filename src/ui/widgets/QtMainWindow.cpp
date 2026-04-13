#include "QtMainWindow.h"

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
#include <QTabWidget>
#include <QLabel>
#include <QStyle>
#include <QVariant>
#include <QGroupBox>
#include <QRadioButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QStatusBar>

#include "control/QtControlWidget.h"
#include "ui/widgets/settings/QtSettingsWidget.h"
#include "ui/widgets/status_bar/QtAppStatusBarWidget.h"
#include "ui/widgets/status_bar/QtMotorDriverStatusBarWidget.h"
#include "ui/widgets/status_bar/QtPressureSensorStatusBarWidget.h"

#include "calibration/result/QtCalibrationResultTableWidget.h"
#include "calibration/recording/QtCalibrationSeriesWidget.h"
#include "calibration/recording/QtCalibrationGraphsWidget.h"
#include "logging/QtLogViewerWidget.h"
#include "video/QtVideoSourceGridWidget.h"
#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/control/ControlViewModel.h"
#include "viewmodels/control/CalibrationSessionControlViewModel.h"

ui::QtMainWindow::QtMainWindow(
    mvvm::MainWindowViewModel& model,
    QWidget* parent
)
    : QMainWindow(parent)
    , model_(model)
{
    /* ================= Dock: Логи ================= */

    createLogDock(model_.logViewerViewModel());
    createCalibrationDock(model_.calibrationSeries());
    createGraphsDock(model_.calibrationSeries());

    auto* viewMenu = menuBar()->addMenu(tr("Диагностика"));
    viewMenu->addAction(log_dock_->toggleViewAction());
    viewMenu->addAction(calibration_dock_->toggleViewAction());
    viewMenu->addAction(graphs_dock_->toggleViewAction());


    /* ================= Central ================= */

    auto* central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);


    /* ================= Левая часть: камеры ================= */

    m_cameras =
        new QtVideoSourceGridWidget(
            "D3D11",
            model_.videoSourceGridViewModel(),
            this
        );


    /* ================= Правая часть ================= */

    auto* rightPanel = new QWidget(central);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);


    /* ================= Tabs ================= */

    tabs_ = new QTabWidget(rightPanel);
    tabs_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    /* =================================================
     * ================= PAGE: ПРОЦЕСС ==================
     * ================================================= */

    auto* processPage = new QWidget();
    auto* processLayout = new QVBoxLayout(processPage);
    processLayout->setContentsMargins(0, 0, 0, 0);
    processLayout->setSpacing(10);


    /* ----- Карточка: таблица ----- */

    // auto* tableCard = new QFrame(processPage);
    // tableCard->setObjectName("contentCard");
    // tableCard->setAttribute(Qt::WA_StyledBackground, true);
    // tableCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //
    // auto* tableLayout = new QVBoxLayout(tableCard);
    // tableLayout->setContentsMargins(12, 12, 12, 12);
    //
    // auto* table = new QTableWidget(8, 16, tableCard);
    // table->horizontalHeader()->setStretchLastSection(true);
    // table->verticalHeader()->setVisible(false);
    // table->setSelectionMode(QAbstractItemView::NoSelection);
    // table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //
    // tableLayout->addWidget(table);

    auto* calibrationResultTable = new QtCalibrationResultTableWidget(model_.calibrationResultTableViewModel());

    /* ----- Карточка: управление ----- */

    auto* controlCard = new QFrame(processPage);
    controlCard->setObjectName("contentCard");
    controlCard->setAttribute(Qt::WA_StyledBackground, true);
    controlCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* controlLayout = new QVBoxLayout(controlCard);
    controlLayout->setContentsMargins(12, 8, 8, 8);
    controlLayout->setSpacing(8);

    auto* controlWidget =
        new QtControlWidget(model_.controlViewModel(), controlCard);

    controlLayout->addWidget(controlWidget, 0, Qt::AlignRight);

    processLayout->addWidget(calibrationResultTable, 1);
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
    settingsCardLayout->setContentsMargins(12, 8, 8, 8);
    settingsCardLayout->setSpacing(8);

    auto* settingsWidget =
        new QtSettingsWidget(model_.settingsViewModel(), settingsCard);

    settingsCardLayout->addWidget(settingsWidget);
    settingsLayout->addWidget(settingsCard);
    settingsLayout->addStretch(1);


    /* ================= Добавление вкладок ================= */

    tabs_->addTab(processPage,  tr("Процесс"));
    settings_tab_index_ = tabs_->addTab(settingsPage, tr("Настройки"));

    rightLayout->addWidget(tabs_, 1);


    /* ================= Status Bar Widgets ================= */

    auto* statusLayout = new QHBoxLayout();
    const auto status_models = model_.statusBarViewModels();

    auto* app_status_bar =
        new QtAppStatusBarWidget(status_models.app_status_bar);


    auto* motor_status_bar =
        new QtMotorDriverStatusBarWidget(status_models.motor_driver_status);

    auto* pressure_status_bar =
        new QtPressureSensorStatusBarWidget(status_models.pressure_sensor_status);


    statusBar()->addWidget(app_status_bar, 0);
    statusBar()->addWidget(motor_status_bar, 0);
    statusBar()->addWidget(pressure_status_bar, 0);

    // statusLayout->addWidget(app_status_bar, 0);
    // statusLayout->addWidget(motor_status_bar, 0);
    // statusLayout->addWidget(pressure_status_bar, 0);
    statusLayout->addStretch();

    rightLayout->addLayout(statusLayout);


    /* ================= Root Layout ================= */

    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(2, 2, 2, 2);
    root->setSpacing(2);

    root->addWidget(m_cameras);
    root->addWidget(rightPanel, 1);

    bindSettingsTabState();
}

ui::QtMainWindow::~QtMainWindow()
{
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

void ui::QtMainWindow::createCalibrationDock(mvvm::CalibrationSeriesViewModel& vm)
{
    calibration_widget_ = new QtCalibrationSeriesWidget(vm);

    calibration_dock_ = new QDockWidget(tr("Журнал калибровки"), this);
    calibration_dock_->setWidget(calibration_widget_);

    calibration_dock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, calibration_dock_);

    calibration_dock_->setFloating(true);
    calibration_dock_->hide();
}


void ui::QtMainWindow::createGraphsDock(mvvm::CalibrationSeriesViewModel& vm)
{
    graphs_widget_ = new QtCalibrationGraphsWidget(vm);

    graphs_dock_ = new QDockWidget(tr("Графики калибровки"), this);
    graphs_dock_->setWidget(graphs_widget_);

    graphs_dock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    addDockWidget(Qt::BottomDockWidgetArea, graphs_dock_);

    graphs_dock_->setFloating(true);
    graphs_dock_->hide();
}

void ui::QtMainWindow::bindSettingsTabState()
{
    auto& calibration_vm = model_.controlViewModel().calibrationViewModel();
    can_start_sub_ = calibration_vm.can_start.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, can_start = change.new_value] {
            if (!tabs_ || settings_tab_index_ < 0) {
                return;
            }

            const bool settings_enabled = can_start;
            tabs_->setTabEnabled(settings_tab_index_, settings_enabled);
            if (!settings_enabled && tabs_->currentIndex() == settings_tab_index_) {
                tabs_->setCurrentIndex(0);
            }
        }, Qt::QueuedConnection);
    }, false);

    const bool settings_enabled = calibration_vm.can_start.get_copy();
    tabs_->setTabEnabled(settings_tab_index_, settings_enabled);
}
