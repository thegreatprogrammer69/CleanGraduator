#include "QtMotorDriverStatusBarWidget.h"

#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include "viewmodels/status_bar/MotorDriverStatusViewModel.h"

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

QtMotorDriverStatusBarWidget::QtMotorDriverStatusBarWidget(
    mvvm::MotorDriverStatusViewModel& vm,
    QWidget* parent
)
    : QWidget(parent)
    , vm_(vm)
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
            font-size: 12px;
        }
        QLabel[role="value"] {
            color: #111827;
            font-size: 13px;
            font-weight: 600;
        }
    )");

    auto* card = new QVBoxLayout(content_card_);
    card->setContentsMargins(12, 10, 12, 10);
    card->setSpacing(8);

    auto* title = new QLabel(QStringLiteral("Драйвер двигателя"), content_card_);
    title->setProperty("role", "value");
    card->addWidget(title);

    auto* form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(6);

    run_value_ = makeValue(content_card_);
    freq_value_ = makeValue(content_card_);
    direction_value_ = makeValue(content_card_);
    home_limit_value_ = makeValue(content_card_);
    end_limit_value_ = makeValue(content_card_);
    fault_value_ = makeValue(content_card_);

    form->addRow(makeCaption(QStringLiteral("Состояние"), content_card_), run_value_);
    form->addRow(makeCaption(QStringLiteral("Частота"), content_card_), freq_value_);
    form->addRow(makeCaption(QStringLiteral("Направление"), content_card_), direction_value_);
    form->addRow(makeCaption(QStringLiteral("Home лимит"), content_card_), home_limit_value_);
    form->addRow(makeCaption(QStringLiteral("End лимит"), content_card_), end_limit_value_);
    form->addRow(makeCaption(QStringLiteral("Ошибка"), content_card_), fault_value_);

    card->addLayout(form);

    root->addWidget(content_card_);

    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this] { refreshAll(); });
    timer_.start();

    refreshAll();
}

QtMotorDriverStatusBarWidget::~QtMotorDriverStatusBarWidget() = default;

void QtMotorDriverStatusBarWidget::refreshAll() {
    const auto limits = vm_.limits();

    run_value_->setText(runToText(vm_.isRunning()));
    freq_value_->setText(QString::number(vm_.frequencyHz()) + QStringLiteral(" Гц"));
    direction_value_->setText(directionToText(vm_.direction()));
    home_limit_value_->setText(boolToText(limits.home));
    end_limit_value_->setText(boolToText(limits.end));
    fault_value_->setText(faultToText(vm_.fault()));
}

QString QtMotorDriverStatusBarWidget::directionToText(domain::common::MotorDirection direction) {
    using D = domain::common::MotorDirection;
    switch (direction) {
        case D::Forward: return QStringLiteral("Вперёд");
        case D::Backward: return QStringLiteral("Назад");
    }

    return QStringLiteral("Неизвестно");
}

QString QtMotorDriverStatusBarWidget::faultToText(domain::common::MotorFault fault) {
    using F = domain::common::MotorFault;
    switch (fault) {
        case F::None: return QStringLiteral("Нет");
        case F::DriverError: return QStringLiteral("Ошибка драйвера");
        case F::EmergencyStop: return QStringLiteral("Аварийный стоп");
    }

    return QStringLiteral("Неизвестно");
}

QString QtMotorDriverStatusBarWidget::runToText(bool is_running) {
    return is_running ? QStringLiteral("Запущен") : QStringLiteral("Остановлен");
}

QString QtMotorDriverStatusBarWidget::boolToText(bool value) {
    return value ? QStringLiteral("Активен") : QStringLiteral("Нет");
}

} // namespace ui
