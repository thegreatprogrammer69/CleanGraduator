#include "QtMotorDriverStatusBarWidget.h"

#include <QFormLayout>
#include <QVariant>
#include <QFrame>
#include <QLabel>
#include <QMetaObject>
#include <QVBoxLayout>

#include <optional>
#include <type_traits>
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

// --- Observable helpers (best-effort) ---

template <class T, class = void>
struct has_get_copy : std::false_type {};

template <class T>
struct has_get_copy<T, std::void_t<decltype(std::declval<const T&>().get_copy())>> : std::true_type {};

template <class Obs>
domain::common::MotorFault readFaultBestEffort(const Obs& obs) {
    using domain::common::MotorFault;

    if constexpr (has_get_copy<Obs>::value) {
        auto v = obs.get_copy();

        if constexpr (std::is_same_v<decltype(v), MotorFault>) {
            return v;
        }
        else if constexpr (std::is_same_v<decltype(v), std::optional<MotorFault>>) {
            return v.value_or(MotorFault{});  // корректно
        }
        else {
            return MotorFault{};              // корректно
        }
    }
    else {
        return MotorFault{};                  // корректно
    }
}


} // namespace

// Храним тип подписки тут, в .cpp (без утечек в заголовок)
struct QtMotorDriverStatusBarWidget::Subscriptions {
    mvvm::Observable<domain::common::MotorFault>::Subscription fault_sub;
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

    // (Опционально) — если у тебя общий QSS, лучше вынести туда.
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

    form->addRow(makeCaption(tr("Состояние"), content_card_), run_value_);
    form->addRow(makeCaption(tr("Частота"),   content_card_), freq_value_);
    form->addRow(makeCaption(tr("Направление"), content_card_), direction_value_);
    form->addRow(makeCaption(tr("Home лимит"),  content_card_), home_limit_value_);
    form->addRow(makeCaption(tr("End лимит"),   content_card_), end_limit_value_);
    form->addRow(makeCaption(tr("Ошибка"),      content_card_), fault_value_);

    card->addLayout(form);
    root->addWidget(content_card_);

    // 1) Подписка на fault (реактивно)
    subs_->fault_sub = vm_.fault.subscribe([this](auto e) {
        // e.new_value — как в твоём примере с логами
        const auto f = e.new_value;
        QMetaObject::invokeMethod(this, [this, f] {
            setFaultText(f);
        }, Qt::QueuedConnection);
    });

    // 2) Остальное — по таймеру (потому что Observable для этих полей нет)
    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this] { refreshAll(); });
    timer_.start();

    // Первичная отрисовка
    refreshAll();
    setFaultText(readFaultBestEffort(vm_.fault));
}

QtMotorDriverStatusBarWidget::~QtMotorDriverStatusBarWidget() {
    timer_.stop();
    subs_.reset(); // отписаться пока QLabel ещё живы
}

void QtMotorDriverStatusBarWidget::refreshAll() {
    const auto limits = vm_.limits();

    run_value_->setText(runToText(vm_.isRunning()));
    freq_value_->setText(QString::number(vm_.frequencyHz()) + tr(" Гц"));
    direction_value_->setText(directionToText(vm_.direction()));
    home_limit_value_->setText(boolToText(limits.home));
    end_limit_value_->setText(boolToText(limits.end));

    // fault обновляется подпиской, но на всякий случай можно и тут синхронизировать:
    // setFaultText(readFaultBestEffort(vm_.fault));
}

void QtMotorDriverStatusBarWidget::setFaultText(
    const domain::common::MotorFault& fault
) {
    fault_value_->setText(faultToText(fault));

    // Можно добавить визуальное состояние
    if (fault.type == domain::common::MotorFaultType::None) {
        fault_value_->setStyleSheet("");
    } else {
        fault_value_->setStyleSheet("color: #B91C1C;"); // красный
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

    QString QtMotorDriverStatusBarWidget::faultToText(
        const domain::common::MotorFault& fault
    ) {
    using T = domain::common::MotorFaultType;

    switch (fault.type) {
        case T::None:
            return tr("Нет");

        case T::DriverError:
            if (!fault.error.empty()) {
                return QString::fromStdString(fault.error);
            }
            return tr("Ошибка драйвера");

        case T::EmergencyStop:
            if (!fault.error.empty()) {
                return QString::fromStdString(fault.error);
            }
            return tr("Аварийный стоп");
    }

    return tr("Неизвестно");
}


QString QtMotorDriverStatusBarWidget::runToText(bool is_running) {
    return is_running ? tr("Запущен") : tr("Остановлен");
}

QString QtMotorDriverStatusBarWidget::boolToText(bool value) {
    return value ? tr("Активен") : tr("Нет");
}

} // namespace ui
