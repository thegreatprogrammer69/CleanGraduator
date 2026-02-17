#include "QtPressureSensorStatusBarWidget.h"

#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QMetaObject>
#include <QVBoxLayout>

#include "viewmodels/status_bar/PressureSensorStatusBarViewModel.h"

namespace ui {

namespace {

QLabel* makeCaption(const QString& text, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setProperty("role", "caption");
    return l;
}

QLabel* makeValue(QWidget* parent) {
    auto* l = new QLabel(parent);
    l->setProperty("role", "value");
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return l;
}

} // namespace

struct QtPressureSensorStatusBarWidget::Subscriptions {
    mvvm::Observable<bool>::Subscription opened_sub;
    mvvm::Observable<domain::common::Pressure>::Subscription pressure_sub;
    mvvm::Observable<std::string>::Subscription error_sub;
};

QtPressureSensorStatusBarWidget::QtPressureSensorStatusBarWidget(
    mvvm::PressureSensorStatusBarViewModel& vm,
    QWidget* parent
)
    : QWidget(parent)
    , vm_(vm)
    , subs_(std::make_unique<Subscriptions>())
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    content_card_ = new QFrame(this);
    content_card_->setObjectName("contentCard");
    content_card_->setFrameShape(QFrame::NoFrame);

    content_card_->setStyleSheet(R"(
        QLabel[role="caption"] {
            color: #6B7280;
            font-size: 9px;
        }
        QLabel[role="value"] {
            color: #111827;
            font-size: 10px;
            font-weight: 550;
        }
    )");

    auto* card = new QVBoxLayout(content_card_);
    card->setContentsMargins(12, 10, 12, 7);
    card->setSpacing(4);

    auto* title = new QLabel(tr("Датчик давления"), content_card_);
    title->setProperty("role", "value");
    card->addWidget(title);

    auto* form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(6);

    opened_value_ = makeValue(content_card_);
    pressure_value_ = makeValue(content_card_);
    pressure_speed_value_ = makeValue(content_card_);
    error_value_ = makeValue(content_card_);

    form->addRow(makeCaption(tr("Состояние"), content_card_), opened_value_);
    form->addRow(makeCaption(tr("Давление"), content_card_), pressure_value_);
    form->addRow(makeCaption(tr("Скорость"), content_card_), pressure_speed_value_);
    form->addRow(makeCaption(tr("Ошибка"), content_card_), error_value_);

    card->addLayout(form);
    root->addWidget(content_card_);

    subs_->opened_sub = vm_.is_opened.subscribe([this](const auto& e) {
        QMetaObject::invokeMethod(this, [this, opened = e.new_value] {
            setOpened(opened);
        }, Qt::QueuedConnection);
    });

    subs_->pressure_sub = vm_.pressure.subscribe([this](const auto& e) {
        QMetaObject::invokeMethod(this, [this, value = e.new_value] {
            setPressure(value);
        }, Qt::QueuedConnection);
    });

    subs_->error_sub = vm_.error.subscribe([this](const auto& e) {
        QMetaObject::invokeMethod(this, [this, text = e.new_value] {
            setError(text);
        }, Qt::QueuedConnection);
    });

    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this] { refreshAll(); });
    timer_.start();

    refreshAll();
}

QtPressureSensorStatusBarWidget::~QtPressureSensorStatusBarWidget() {
    timer_.stop();
    subs_.reset();
}

void QtPressureSensorStatusBarWidget::refreshAll() {
    pressure_speed_value_->setText(pressureSpeedToText(vm_.pressureSpeedPaPerSecond()));
}

void QtPressureSensorStatusBarWidget::setOpened(bool opened) {
    opened_value_->setText(boolToText(opened));
}

void QtPressureSensorStatusBarWidget::setPressure(const domain::common::Pressure& value) {
    pressure_value_->setText(pressureToText(value));
}

void QtPressureSensorStatusBarWidget::setError(const std::string& error_text) {
    if (error_text.empty()) {
        error_value_->setText(tr("Нет"));
        error_value_->setStyleSheet("");
        return;
    }

    error_value_->setText(QString::fromStdString(error_text));
    error_value_->setStyleSheet("color: #B91C1C;");
}

QString QtPressureSensorStatusBarWidget::boolToText(bool value) {
    return value ? tr("Открыт") : tr("Закрыт");
}

QString QtPressureSensorStatusBarWidget::pressureToText(const domain::common::Pressure& value) {
    return QString::number(value.kpa(), 'f', 3) + tr(" кПа");
}

QString QtPressureSensorStatusBarWidget::pressureSpeedToText(double pa_per_sec) {
    return QString::number(pa_per_sec, 'f', 2) + tr(" Па/с");
}

}
