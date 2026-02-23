#include "QtMotorDriverStatusBarWidget.h"

#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QVariant>
#include <QMetaObject>
#include <QVBoxLayout>

#include <utility>

#include "viewmodels/status_bar/MotorDriverStatusViewModel.h"

namespace ui {

namespace {

// --- UI helpers ---

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

// Храним типы подписок тут, в .cpp
struct QtMotorDriverStatusBarWidget::Subscriptions {
    mvvm::Observable<bool>::Subscription running_sub;
    mvvm::Observable<std::string>::Subscription error_sub;
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

    auto* title = new QLabel(tr("Драйвер двигателя"), content_card_);
    title->setProperty("role", "value");
    card->addWidget(title);

    auto* form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(6);

    run_value_        = makeValue(content_card_);
    freq_value_       = makeValue(content_card_);
    direction_value_  = makeValue(content_card_);
    home_limit_value_ = makeValue(content_card_);
    end_limit_value_  = makeValue(content_card_);
    fault_value_      = makeValue(content_card_);

    form->addRow(makeCaption(tr("Состояние"),    content_card_), run_value_);
    form->addRow(makeCaption(tr("Частота"),      content_card_), freq_value_);
    form->addRow(makeCaption(tr("Направление"),  content_card_), direction_value_);
    form->addRow(makeCaption(tr("Home лимит"),   content_card_), home_limit_value_);
    form->addRow(makeCaption(tr("End лимит"),    content_card_), end_limit_value_);
    form->addRow(makeCaption(tr("Ошибка"),       content_card_), fault_value_);

    card->addLayout(form);
    root->addWidget(content_card_);

    // --- Reactive bindings ---

    subs_->running_sub = vm_.is_running_.subscribe([this](const auto& e) {
        const bool v = e.new_value;
        QMetaObject::invokeMethod(this, [this, v] { setRunning(v); }, Qt::QueuedConnection);
    });

    subs_->error_sub = vm_.error_.subscribe([this](const auto& e) {
        const auto s = e.new_value; // std::string (копия)
        QMetaObject::invokeMethod(this, [this, s] { setError(s); }, Qt::QueuedConnection);
    });

    subs_->direction_sub = vm_.direction_.subscribe([this](const auto& e) {
        const auto d = e.new_value;
        QMetaObject::invokeMethod(this, [this, d] { setDirection(d); }, Qt::QueuedConnection);
    });

    subs_->limits_sub = vm_.limits_state_.subscribe([this](const auto& e) {
        const auto l = e.new_value;
        QMetaObject::invokeMethod(this, [this, l] { setLimits(l); }, Qt::QueuedConnection);
    });

    // --- Frequency polling (если frequency() не Observable) ---
    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this] { refreshFrequency(); });
    timer_.start();

    // --- Initial sync ---
    initializeFromViewModel();
}

QtMotorDriverStatusBarWidget::~QtMotorDriverStatusBarWidget() {
    timer_.stop();
    subs_.reset(); // отписаться пока QLabel ещё живы
}

void QtMotorDriverStatusBarWidget::refreshFrequency() {
    freq_value_->setText(QString::number(vm_.frequency()) + tr(" Гц"));
}

void QtMotorDriverStatusBarWidget::setRunning(bool is_running) {
    run_value_->setText(runToText(is_running));
}

void QtMotorDriverStatusBarWidget::setDirection(domain::common::MotorDirection direction) {
    direction_value_->setText(directionToText(direction));
}

void QtMotorDriverStatusBarWidget::setLimits(domain::common::MotorLimitsState limits) {
    home_limit_value_->setText(boolToText(limits.home));
    end_limit_value_->setText(boolToText(limits.end));
}

void QtMotorDriverStatusBarWidget::setError(const std::string& error) {
    fault_value_->setText(errorToText(error));

    if (error.empty()) {
        fault_value_->setStyleSheet("");
    } else {
        fault_value_->setStyleSheet("color: #B91C1C;");
    }
}

QString QtMotorDriverStatusBarWidget::directionToText(domain::common::MotorDirection direction) {
    using D = domain::common::MotorDirection;
    switch (direction) {
        case D::Forward:  return tr("Вперёд");
        case D::Backward: return tr("Назад");
    }
    return tr("Неизвестно");
}

QString QtMotorDriverStatusBarWidget::runToText(bool is_running) {
    return is_running ? tr("Запущен") : tr("Остановлен");
}

QString QtMotorDriverStatusBarWidget::boolToText(bool value) {
    return value ? tr("Активен") : tr("Нет");
}

QString QtMotorDriverStatusBarWidget::errorToText(const std::string& error) {
    if (error.empty()) return tr("Нет");
    return QString::fromStdString(error);
}

void QtMotorDriverStatusBarWidget::initializeFromViewModel()
{
    // Если Observable поддерживает get_copy()
    setRunning(vm_.is_running_.get_copy());
    setError(vm_.error_.get_copy());
    setDirection(vm_.direction_.get_copy());
    setLimits(vm_.limits_state_.get_copy());

    refreshFrequency();
}

} // namespace ui