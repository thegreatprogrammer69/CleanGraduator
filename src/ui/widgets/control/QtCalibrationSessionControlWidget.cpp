#include "QtCalibrationSessionControlWidget.h"

#include <QGridLayout>
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
    mainLayout->setContentsMargins(0, 0, 0, 0);

    auto* buttonLayout = new QHBoxLayout();
    startButton_ = new QPushButton(tr("Запуск"), this);
    stopButton_ = new QPushButton(tr("Стоп"), this);
    aimButton_ = new QPushButton(tr("Прицел"), this);

    buttonLayout->addWidget(startButton_);
    buttonLayout->addWidget(stopButton_);
    buttonLayout->addWidget(aimButton_);

    backwardCheckBox_ = new QCheckBox(tr("обр. ход"), this);
    slowdownCheckBox_ = new QCheckBox(tr("замедление"), this);
    playValveCheckBox_ = new QCheckBox(tr("\"играть\" клапаном"), this);
    kuCheckBox_ = new QCheckBox(tr("к. у."), this);
    centeredMarkCheckBox_ = new QCheckBox(tr("центр. риску"), this);

    auto* checks = new QGridLayout();
    checks->addWidget(backwardCheckBox_, 0, 0);
    checks->addWidget(kuCheckBox_, 0, 1);
    checks->addWidget(slowdownCheckBox_, 1, 0);
    checks->addWidget(centeredMarkCheckBox_, 1, 1);
    checks->addWidget(playValveCheckBox_, 2, 0, 1, 2);

    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet("color: #b91c1c;");
    errorLabel_->setWordWrap(true);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(checks);
    mainLayout->addWidget(errorLabel_);
}

void QtCalibrationSessionControlWidget::bind() {
    connect(backwardCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        vm_.setBackwardStrokeEnabled(checked);
    });
    connect(slowdownCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        vm_.setSlowdownEnabled(checked);
    });
    connect(playValveCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        vm_.setPlayValveEnabled(checked);
    });
    connect(kuCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        vm_.setKuModeEnabled(checked);
    });
    connect(centeredMarkCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        vm_.setCenteredMarkEnabled(checked);
    });

    connect(startButton_, &QPushButton::clicked, this, [this] { vm_.startCalibration(); });
    connect(aimButton_, &QPushButton::clicked, this, [this] { vm_.startAim(); });
    connect(stopButton_, &QPushButton::clicked, this, [this] { vm_.emergencyStop(); });

    errorSub_ = vm_.error_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)]() {
            errorLabel_->setText(text);
        }, Qt::QueuedConnection);
    }, false);

    kuModeSub_ = vm_.ku_mode_enabled.subscribe([this](const auto& c) { kuCheckBox_->setChecked(c.new_value); }, false);
    centeredMarkSub_ = vm_.centered_mark_enabled.subscribe([this](const auto& c) { centeredMarkCheckBox_->setChecked(c.new_value); }, false);
    backwardSub_ = vm_.backward_stroke_enabled.subscribe([this](const auto& c) { backwardCheckBox_->setChecked(c.new_value); }, false);
    slowdownSub_ = vm_.slowdown_enabled.subscribe([this](const auto& c) { slowdownCheckBox_->setChecked(c.new_value); }, false);
    playValveSub_ = vm_.play_valve_enabled.subscribe([this](const auto& c) { playValveCheckBox_->setChecked(c.new_value); }, false);

    canStartSub_ = vm_.can_start.subscribe([this](const auto& c) {
        QMetaObject::invokeMethod(this, [this, value = c.new_value]() {
            startButton_->setEnabled(value);
            aimButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);

    canStopSub_ = vm_.can_stop.subscribe([this](const auto& c) {
        QMetaObject::invokeMethod(this, [this, value = c.new_value]() {
            stopButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);
}

} // namespace ui
