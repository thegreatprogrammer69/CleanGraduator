#include "QtMotorControlWidget.h"

#include "viewmodels/control/MotorControlViewModel.h"

namespace ui {

QtMotorControlWidget::QtMotorControlWidget(
    mvvm::MotorControlViewModel& vm,
    QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
    bind();
}

QtMotorControlWidget::~QtMotorControlWidget() = default;

void QtMotorControlWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    statusLabel_ = new QLabel(tr("Простаивает"), this);
    statusLabel_->setAlignment(Qt::AlignCenter);

    frequencySpin_ = new QSpinBox(this);
    frequencySpin_->setRange(0, 100000);
    frequencySpin_->setValue(1000);

    directionBox_ = new QComboBox(this);
    directionBox_->addItem(tr("Вперёд"));
    directionBox_->addItem(tr("Назад"));

    startButton_ = new QPushButton(tr("Старт"), this);
    stopButton_  = new QPushButton(tr("Стоп"), this);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(startButton_);
    btnLayout->addWidget(stopButton_);

    mainLayout->addWidget(new QLabel(tr("Частота:"), this));
    mainLayout->addWidget(frequencySpin_);
    mainLayout->addWidget(new QLabel(tr("Направление:"), this));
    mainLayout->addWidget(directionBox_);
    mainLayout->addWidget(statusLabel_);
    mainLayout->addLayout(btnLayout);
}

void QtMotorControlWidget::bind()
{
    // UI → ViewModel
    connect(startButton_, &QPushButton::clicked, this, [this] {
        vm_.setFrequency(frequencySpin_->value());

        auto dir = directionBox_->currentIndex() == 0
            ? domain::common::MotorDirection::Forward
            : domain::common::MotorDirection::Backward;

        vm_.setDirection(dir);
        vm_.start();
    });

    connect(stopButton_, &QPushButton::clicked, this, [this] {
        vm_.stop();
    });

    // ViewModel → UI
    runningSub_ = vm_.is_running.subscribe(
        [this](const auto& running)
        {
            const bool r = running.new_value;

            QMetaObject::invokeMethod(
                this,
                [this, r]()
                {
                    statusLabel_->setText(r ? tr("В движении")
                                            : tr("Простаивает"));

                    startButton_->setEnabled(!r);
                    stopButton_->setEnabled(r);
                },
                Qt::QueuedConnection
            );
        }
    );
}

} // namespace ui
