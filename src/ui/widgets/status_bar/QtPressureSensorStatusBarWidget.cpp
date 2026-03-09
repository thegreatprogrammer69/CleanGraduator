#include "QtPressureSensorStatusBarWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>

#include <sstream>

#include "viewmodels/status_bar/PressureSensorStatusBarViewModel.h"

#include "domain/fmt/fmt.h"

namespace ui {

static QLabel* makeSeparator(QWidget* parent)
{
    return new QLabel("|", parent);
}

struct QtPressureSensorStatusBarWidget::Subscriptions {
    mvvm::Observable<bool>::Subscription opened_sub;
    mvvm::Observable<domain::common::Pressure>::Subscription pressure_sub;
};

QtPressureSensorStatusBarWidget::QtPressureSensorStatusBarWidget(
    mvvm::PressureSensorStatusBarViewModel& vm,
    QWidget* parent
)
    : QWidget(parent)
    , vm_(vm)
    , subs_(std::make_unique<Subscriptions>())
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4,0,4,0);
    layout->setSpacing(6);

    opened_value_   = new QLabel(this);
    pressure_value_ = new QLabel(this);
    speed_value_    = new QLabel(this);

    opened_value_->setMinimumWidth(160);
    pressure_value_->setMinimumWidth(90);
    speed_value_->setMinimumWidth(110);

    layout->addWidget(opened_value_);
    layout->addWidget(makeSeparator(this));
    layout->addWidget(pressure_value_);
    layout->addWidget(makeSeparator(this));
    layout->addWidget(speed_value_);

    // subscriptions

    subs_->opened_sub = vm_.is_opened.subscribe([this](auto e) {
        const bool opened = e.new_value;
        QMetaObject::invokeMethod(this,
            [this, opened]{ setOpened(opened); },
            Qt::QueuedConnection);
    });

    subs_->pressure_sub = vm_.pressure.subscribe([this](auto) {
        QMetaObject::invokeMethod(this,
            [this]{ setPressure(); },
            Qt::QueuedConnection);
    }, false);

    timer_.setInterval(100);
    connect(&timer_, &QTimer::timeout, this, [this]{ refreshAll(); });
    timer_.start();

    // initial sync
    setOpened(vm_.is_opened.get_copy());
    setPressure();
    setSpeed();
}

QtPressureSensorStatusBarWidget::~QtPressureSensorStatusBarWidget()
{
    timer_.stop();
    subs_.reset();
}

QString QtPressureSensorStatusBarWidget::openedToText(bool opened)
{
    return opened ? tr("Датчик давл. Активен")
                  : tr("Датчик давл. Остановлен");
}

void QtPressureSensorStatusBarWidget::setOpened(bool opened)
{
    opened_value_->setText(openedToText(opened));
}

void QtPressureSensorStatusBarWidget::setPressure()
{
    const auto p = vm_.pressure.get_copy();

    std::stringstream ss;
    ss << p;

    pressure_value_->setText(QString::fromStdString(ss.str()));
}

void QtPressureSensorStatusBarWidget::setSpeed()
{
    const auto speed = vm_.pressureSpeedPerSecond();

    std::stringstream ss;
    ss << speed << " / sec";

    auto text = QString::fromStdString(ss.str());

    if (text[0] != '-') text = " " + text;

    speed_value_->setText(text);
}

void QtPressureSensorStatusBarWidget::refreshAll()
{
    setSpeed();
}

}