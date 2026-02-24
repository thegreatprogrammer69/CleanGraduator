#include "QtCalibrationSessionControlWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "viewmodels/control/CalibrationSessionControlViewModel.h"

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

void QtCalibrationSessionControlWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);

    errorLabel_ = new QLabel(this);
    errorLabel_->setText(tr(""));
    errorLabel_->setStyleSheet("color: #b91c1c; font-weight: 600;");
    errorLabel_->setWordWrap(true);

    modeBox_ = new QComboBox(this);
    modeBox_->addItem(tr("Полная калибровка"));
    modeBox_->addItem(tr("Только прямой ход"));
    modeBox_->addItem(tr("Только последняя точка"));

    startButton_ = new QPushButton(tr("Старт"), this);
    stopButton_ = new QPushButton(tr("Стоп"), this);
    emergencyStopButton_ = new QPushButton(tr("Экстренный стоп"), this);

    auto* modeLabel = new QLabel(tr("Режим калибровки:"), this);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(startButton_);
    buttonLayout->addWidget(stopButton_);
    buttonLayout->addWidget(emergencyStopButton_);

    mainLayout->addWidget(errorLabel_);
    mainLayout->addWidget(modeLabel);
    mainLayout->addWidget(modeBox_);
    mainLayout->addLayout(buttonLayout);
}

void QtCalibrationSessionControlWidget::bind() {
    connect(modeBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        switch (idx) {
        case 1:
            vm_.setCalibrationMode(domain::common::CalibrationMode::OnlyForward);
            break;
        case 2:
            vm_.setCalibrationMode(domain::common::CalibrationMode::OnlyLast);
            break;
        default:
            vm_.setCalibrationMode(domain::common::CalibrationMode::Full);
            break;
        }
    });

    connect(startButton_, &QPushButton::clicked, this, [this] {
        vm_.startCalibration();
    });

    connect(stopButton_, &QPushButton::clicked, this, [this] {
        vm_.stopCalibration();
    });

    connect(emergencyStopButton_, &QPushButton::clicked, this, [this] {
        vm_.emergencyStop();
    });

    errorSub_ = vm_.error_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(
            this,
            [this, text = QString::fromStdString(change.new_value)]() {
                errorLabel_->setText(text);
            },
            Qt::QueuedConnection);
    }, false);

    canStartSub_ = vm_.can_start.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(
            this,
            [this, value = change.new_value]() {
                startButton_->setEnabled(value);
            },
            Qt::QueuedConnection);
    }, false);

    canStopSub_ = vm_.can_stop.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(
            this,
            [this, value = change.new_value]() {
                stopButton_->setEnabled(value);
            },
            Qt::QueuedConnection);
    }, false);

    canAbortSub_ = vm_.can_abort.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(
            this,
            [this, value = change.new_value]() {
                emergencyStopButton_->setEnabled(value);
            },
            Qt::QueuedConnection);
    }, false);

    errorLabel_->setText(QString::fromStdString(vm_.error_text.get_copy()));
    startButton_->setEnabled(vm_.can_start.get_copy());
    stopButton_->setEnabled(vm_.can_stop.get_copy());
    emergencyStopButton_->setEnabled(vm_.can_abort.get_copy());
}

} // namespace ui
