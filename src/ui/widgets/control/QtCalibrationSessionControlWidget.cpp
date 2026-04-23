#include "QtCalibrationSessionControlWidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMetaObject>
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

    reverseModeCheckBox_ = new QCheckBox(tr("обр. ход"), this);
    slowdownCheckBox_ = new QCheckBox(tr("замедление"), this);
    playValveCheckBox_ = new QCheckBox(tr("\"играть\" клапаном"), this);
    kuCheckBox_ = new QCheckBox(tr("к. у."), this);
    centeredMarkCheckBox_ = new QCheckBox(tr("центр. риску"), this);

    auto* optionsGrid = new QGridLayout();
    optionsGrid->addWidget(reverseModeCheckBox_, 0, 0);
    optionsGrid->addWidget(kuCheckBox_, 0, 1);
    optionsGrid->addWidget(slowdownCheckBox_, 1, 0);
    optionsGrid->addWidget(centeredMarkCheckBox_, 1, 1);
    optionsGrid->addWidget(playValveCheckBox_, 2, 0, 1, 2);

    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet("color: #b91c1c;");
    errorLabel_->setWordWrap(true);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(optionsGrid);
    mainLayout->addWidget(errorLabel_);
}

void QtCalibrationSessionControlWidget::bind() {
    connect(reverseModeCheckBox_, &QCheckBox::toggled, this, [this](bool checked) {
        vm_.setReverseModeEnabled(checked);
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

    connect(startButton_, &QPushButton::clicked, this, [this] {
        vm_.startCalibration();
    });
    connect(stopButton_, &QPushButton::clicked, this, [this] {
        vm_.emergencyStop();
    });
    connect(aimButton_, &QPushButton::clicked, this, [this] {
        vm_.aimCalibration();
    });

    errorSub_ = vm_.error_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)]() {
            errorLabel_->setText(text);
            errorLabel_->setVisible(!text.isEmpty());
        }, Qt::QueuedConnection);
    }, false);

    auto bindBool = [this](auto& sub, auto& obs, QCheckBox* box) {
        sub = obs.subscribe([this, box](const auto& change) {
            QMetaObject::invokeMethod(this, [box, value = change.new_value]() {
                box->setChecked(value);
            }, Qt::QueuedConnection);
        }, false);
    };

    bindBool(reverseModeSub_, vm_.reverse_mode_enabled, reverseModeCheckBox_);
    bindBool(slowdownSub_, vm_.slowdown_enabled, slowdownCheckBox_);
    bindBool(playValveSub_, vm_.play_valve_enabled, playValveCheckBox_);
    bindBool(kuModeSub_, vm_.ku_mode_enabled, kuCheckBox_);
    bindBool(centeredMarkSub_, vm_.centered_mark_enabled, centeredMarkCheckBox_);

    canStartSub_ = vm_.can_start.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value]() {
            startButton_->setEnabled(value);
            aimButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);

    canStopSub_ = vm_.can_stop.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, value = change.new_value]() {
            stopButton_->setEnabled(value);
        }, Qt::QueuedConnection);
    }, false);

    errorLabel_->setText(QString::fromStdString(vm_.error_text.get_copy()));
    errorLabel_->setVisible(!errorLabel_->text().isEmpty());
    reverseModeCheckBox_->setChecked(vm_.reverse_mode_enabled.get_copy());
    slowdownCheckBox_->setChecked(vm_.slowdown_enabled.get_copy());
    playValveCheckBox_->setChecked(vm_.play_valve_enabled.get_copy());
    kuCheckBox_->setChecked(vm_.ku_mode_enabled.get_copy());
    centeredMarkCheckBox_->setChecked(vm_.centered_mark_enabled.get_copy());
    startButton_->setEnabled(vm_.can_start.get_copy());
    aimButton_->setEnabled(vm_.can_start.get_copy());
    stopButton_->setEnabled(vm_.can_stop.get_copy());
}

} // namespace ui
