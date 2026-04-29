#include "QtMotorDriverStatusBarWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>

#include "viewmodels/status_bar/MotorDriverStatusViewModel.h"

namespace ui {

static QLabel* makeSeparator(QWidget* parent)
{
    return new QLabel("|", parent);
}

struct QtMotorDriverStatusBarWidget::Subscriptions {
    mvvm::Observable<bool>::Subscription running_sub;
    mvvm::Observable<domain::common::MotorDirection>::Subscription direction_sub;
    mvvm::Observable<domain::common::MotorLimitsState>::Subscription limits_sub;
};

QtMotorDriverStatusBarWidget::QtMotorDriverStatusBarWidget(
    mvvm::MotorDriverStatusViewModel& vm,
    QWidget* parent
)
    : QWidget(parent)
    , vm_(vm)
    , subs_(std::make_unique<Subscriptions>())
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4,0,4,0);
    layout->setSpacing(6);

    running_value_ = new QLabel(this);
    motion_value_  = new QLabel(this);

    home_value_ = new QLabel("Home", this);
    end_value_  = new QLabel("End", this);

    running_value_->setMinimumWidth(150);
    motion_value_->setMinimumWidth(90);

    home_value_->setMinimumWidth(40);
    end_value_->setMinimumWidth(40);

    layout->addWidget(running_value_);

    layout->addWidget(makeSeparator(this));

    layout->addWidget(motion_value_);

    layout->addWidget(makeSeparator(this));

    layout->addWidget(home_value_);
    layout->addWidget(end_value_);

    // subscriptions

    subs_->running_sub = vm_.is_running_.subscribe([this](const auto& e) {
        const bool v = e.new_value;
        QMetaObject::invokeMethod(this, [this,v]{ setRunning(v); }, Qt::QueuedConnection);
    });

    subs_->direction_sub = vm_.direction_.subscribe([this](const auto& e) {
        const auto d = e.new_value;
        QMetaObject::invokeMethod(this, [this,d]{ setDirection(d); }, Qt::QueuedConnection);
    });

    subs_->limits_sub = vm_.limits_state_.subscribe([this](const auto& e) {
        const auto l = e.new_value;
        QMetaObject::invokeMethod(this, [this,l]{ setLimits(l); }, Qt::QueuedConnection);
    });

    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this]{ refreshFrequency(); });
    timer_.start();

    initializeFromViewModel();
}

QtMotorDriverStatusBarWidget::~QtMotorDriverStatusBarWidget()
{
    timer_.stop();
    subs_.reset();
}

QString QtMotorDriverStatusBarWidget::directionArrow(domain::common::MotorDirection direction)
{
    using D = domain::common::MotorDirection;

    switch (direction) {
        case D::Forward:  return "→";
        case D::Backward: return "←";
    }

    return "?";
}

void QtMotorDriverStatusBarWidget::setRunning(bool running)
{
    running_value_->setText(
        running ? tr("Двиг. активен") : tr("Двиг. остановлен")
    );
}

void QtMotorDriverStatusBarWidget::refreshFrequency()
{
    const auto freq = vm_.frequency();
    const auto arrow = directionArrow(vm_.direction_.get_copy());

    motion_value_->setText(
        QString("%1 %2 Гц").arg(arrow).arg(freq)
    );
}

void QtMotorDriverStatusBarWidget::setDirection(domain::common::MotorDirection direction)
{
    const auto freq = vm_.frequency();

    motion_value_->setText(
        QString("%1 %2 Гц")
        .arg(directionArrow(direction))
        .arg(freq)
    );
}

void QtMotorDriverStatusBarWidget::setLimits(domain::common::MotorLimitsState limits)
{
    home_value_->setStyleSheet(
        limits.home ? "color: #dc2626;" : "color: #334155;"
    );

    end_value_->setStyleSheet(
        limits.end ? "color: #dc2626;" : "color: #334155;"
    );
}

void QtMotorDriverStatusBarWidget::initializeFromViewModel()
{
    setRunning(vm_.is_running_.get_copy());
    setDirection(vm_.direction_.get_copy());
    setLimits(vm_.limits_state_.get_copy());

    refreshFrequency();
}

}