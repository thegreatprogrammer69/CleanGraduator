#include "QtPressureSensorStatusBarWidget.h"

#include <QFormLayout>
#include <QVariant>
#include <QFrame>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QVBoxLayout>

#include "domain/fmt/fmt.h"
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
    mvvm::Observable<bool>::Subscription is_opened_sub;
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
    speed_value_ = makeValue(content_card_);
    error_value_ = makeValue(content_card_);

    form->addRow(makeCaption(tr("Состояние"), content_card_), opened_value_);
    form->addRow(makeCaption(tr("Давление"), content_card_), pressure_value_);
    form->addRow(makeCaption(tr("Скорость"), content_card_), speed_value_);
    form->addRow(makeCaption(tr("Ошибка"), content_card_), error_value_);

    card->addLayout(form);

    action_button_ = new QPushButton(content_card_);
    action_button_->setProperty("role", "action");
    action_button_->setCursor(Qt::PointingHandCursor);
    connect(action_button_, &QPushButton::clicked, this, [this] {
        if (vm_.is_opened.get_copy()) {
            vm_.close();
            return;
        }

        vm_.open();
    });
    card->addWidget(action_button_, 0, Qt::AlignRight);

    root->addWidget(content_card_);

    subs_->is_opened_sub = vm_.is_opened.subscribe([this](auto e) {
        const bool opened = e.new_value;
        QMetaObject::invokeMethod(this, [this, opened] {
            setOpenedText(opened);
            setActionButtonText(opened);
        }, Qt::QueuedConnection);
    });

    subs_->pressure_sub = vm_.pressure.subscribe([this](auto) {
        QMetaObject::invokeMethod(this, [this] { setPressureText(); }, Qt::QueuedConnection);
    }, false);

    subs_->error_sub = vm_.error.subscribe([this](auto e) {
        const auto err = e.new_value;
        QMetaObject::invokeMethod(this, [this, err] { setErrorText(err); }, Qt::QueuedConnection);
    });

    timer_.setInterval(100);
    connect(&timer_, &QTimer::timeout, this, [this] { refreshAll(); });
    timer_.start();

    refreshAll();
    setOpenedText(vm_.is_opened.get_copy());
    setActionButtonText(vm_.is_opened.get_copy());
    setPressureText();
    setErrorText(vm_.error.get_copy());
}

QtPressureSensorStatusBarWidget::~QtPressureSensorStatusBarWidget() {
    timer_.stop();
    subs_.reset();
}

QString QtPressureSensorStatusBarWidget::openedToText(bool is_opened) {
    return is_opened ? tr("Открыт") : tr("Закрыт");
}

QString QtPressureSensorStatusBarWidget::actionToText(bool is_opened) {
    return is_opened ? tr("Закрыть") : tr("Открыть");
}

QString QtPressureSensorStatusBarWidget::errorToText(const std::string& err) {
    if (err.empty()) {
        return tr("Нет");
    }

    return QString::fromStdString(err);
}

void QtPressureSensorStatusBarWidget::refreshAll() {
    const auto speed = vm_.pressureSpeedPerSecond();
    std::stringstream ss;
    ss << speed;
    auto val = QString::fromStdString(ss.str() + " / sec");
    if (val[0] != '-') val = " " + val;
    speed_value_->setText(val);
}

void QtPressureSensorStatusBarWidget::setOpenedText(bool is_opened) {
    opened_value_->setText(openedToText(is_opened));
}

void QtPressureSensorStatusBarWidget::setActionButtonText(bool is_opened) {
    action_button_->setText(actionToText(is_opened));
}

void QtPressureSensorStatusBarWidget::setPressureText() {
    const auto p = vm_.pressure.get_copy();
    std::stringstream ss;
    ss << p;
    pressure_value_->setText(QString::fromStdString(ss.str()));
}

void QtPressureSensorStatusBarWidget::setErrorText(const std::string& err) {
    error_value_->setText(errorToText(err));

    if (err.empty()) {
        error_value_->setStyleSheet("");
        return;
    }

    error_value_->setStyleSheet("color: #B91C1C;");
}

} // namespace ui
