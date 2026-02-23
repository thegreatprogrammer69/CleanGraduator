#include "QtCalibrationSessionControlWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "domain/core/calibration/common/CalibrationMode.h"
#include "viewmodels/control/CalibrationSessionControlViewModel.h"

namespace {

domain::common::CalibrationMode modeByIndex(int index)
{
    using M = domain::common::CalibrationMode;
    switch (index) {
        case 1:
            return M::OnlyForward;
        case 2:
            return M::OnlyLast;
        case 0:
        default:
            return M::Full;
    }
}

} // namespace

namespace ui {

QtCalibrationSessionControlWidget::QtCalibrationSessionControlWidget(
    mvvm::CalibrationSessionControlViewModel& vm,
    QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
    bind();
}

QtCalibrationSessionControlWidget::~QtCalibrationSessionControlWidget() = default;

void QtCalibrationSessionControlWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    errorLabel_ = new QLabel(this);
    errorLabel_->setWordWrap(true);
    errorLabel_->setStyleSheet("color: #DC2626; font-weight: 600;");
    errorLabel_->setText(tr(""));

    modeCombo_ = new QComboBox(this);
    modeCombo_->addItem(tr("Полная калибровка"));
    modeCombo_->addItem(tr("Только прямой ход"));
    modeCombo_->addItem(tr("Только последняя точка"));

    startButton_ = new QPushButton(tr("Старт"), this);
    stopButton_ = new QPushButton(tr("Стоп"), this);
    emergencyStopButton_ = new QPushButton(tr("Экстренный стоп"), this);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(startButton_);
    buttons->addWidget(stopButton_);
    buttons->addWidget(emergencyStopButton_);

    mainLayout->addWidget(new QLabel(tr("Ошибка:"), this));
    mainLayout->addWidget(errorLabel_);
    mainLayout->addWidget(new QLabel(tr("Режим калибровки:"), this));
    mainLayout->addWidget(modeCombo_);
    mainLayout->addLayout(buttons);
}

void QtCalibrationSessionControlWidget::bind()
{
    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        vm_.setCalibrationMode(modeByIndex(idx));
    });

    connect(startButton_, &QPushButton::clicked, this, [this] {
        vm_.start();
    });

    connect(stopButton_, &QPushButton::clicked, this, [this] {
        vm_.stop();
    });

    connect(emergencyStopButton_, &QPushButton::clicked, this, [this] {
        vm_.emergencyStop();
    });

    errorSub_ = vm_.error_text.subscribe([this](const auto& change) {
        errorLabel_->setText(QString::fromStdString(change.new_value));
    }, false);

    runningSub_ = vm_.is_running.subscribe([this](const auto& change) {
        const bool running = change.new_value;
        startButton_->setEnabled(!running);
        stopButton_->setEnabled(running);
    }, false);
}

} // namespace ui
