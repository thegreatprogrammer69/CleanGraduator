#include "QtPressureSensorStatusBarWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>

#include "viewmodels/status_bar/PressureSensorStatusBarViewModel.h"

namespace ui {

struct QtPressureSensorStatusBarWidget::Subscriptions {
    mvvm::Observable<bool>::Subscription opened_sub;
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

    opened_value_ = new QLabel(this);
    opened_value_->setMinimumWidth(200);
    layout->addWidget(opened_value_);

    subs_->opened_sub = vm_.is_opened.subscribe([this](auto e) {
        const bool opened = e.new_value;
        QMetaObject::invokeMethod(this,
            [this, opened]{ setOpened(opened); },
            Qt::QueuedConnection);
    });

    setOpened(vm_.is_opened.get_copy());
}

QtPressureSensorStatusBarWidget::~QtPressureSensorStatusBarWidget() = default;

QString QtPressureSensorStatusBarWidget::openedToText(bool opened)
{
    return opened ? tr("Датчик давления активен")
                  : tr("Датчик давления остановлен");
}

void QtPressureSensorStatusBarWidget::setOpened(bool opened)
{
    opened_value_->setText(openedToText(opened));
}

} // namespace ui
