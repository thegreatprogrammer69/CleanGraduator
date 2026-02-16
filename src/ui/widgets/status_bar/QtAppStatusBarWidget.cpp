// ui/widgets/QtAppStatusBarWidget.cpp
#include "QtAppStatusBarWidget.h"

#include <QFrame>
#include <QLabel>
#include <QVariant>
#include <QVBoxLayout>
#include <QFormLayout>

#include <chrono>

#include "viewmodels/status_bar/AppStatusBarViewModel.h"

namespace ui {

static QLabel* makeCaption(const QString& text, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setProperty("role", "caption");
    return l;
}

static QLabel* makeValue(QWidget* parent) {
    auto* l = new QLabel(parent);
    l->setProperty("role", "value");
    l->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return l;
}

static QLabel* makeTimeValue(QWidget* parent) {
    auto* l = makeValue(parent);
    l->setProperty("role", "timeValue");
    return l;
}

QtAppStatusBarWidget::QtAppStatusBarWidget(mvvm::AppStatusBarViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    content_card_ = new QFrame(this);
    content_card_->setObjectName("contentCard");
    content_card_->setFrameShape(QFrame::NoFrame);

    // Базовый стиль карточки + типографика (можно перенести в общий .qss)
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
        QLabel[role="timeValue"] {
            font-family: "Consolas", "DejaVu Sans Mono", monospace;
            letter-spacing: 1px;
        }
        QLabel[role="statusBadge"] {
            padding: 3px 8px;
            border-radius: 8px;
            font-weight: 700;
        }
    )");

    auto* card = new QVBoxLayout(content_card_);
    card->setContentsMargins(12, 10, 12, 10);
    card->setSpacing(8);

    // Верхняя строка: "Статус" + badge
    {
        auto* row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(8);

        auto* caption = makeCaption(QStringLiteral("Статус"), content_card_);
        statusBadge_ = new QLabel(content_card_);
        statusBadge_->setProperty("role", "statusBadge");
        statusBadge_->setTextInteractionFlags(Qt::TextSelectableByMouse);

        row->addWidget(caption);
        row->addWidget(statusBadge_);
        row->addStretch();

        card->addLayout(row);
    }

    // Нижние 2 строки ровно по колонкам: FormLayout
    {
        auto* form = new QFormLayout();
        form->setContentsMargins(0, 0, 0, 0);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(6);
        form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        form->setFormAlignment(Qt::AlignTop);
        form->setRowWrapPolicy(QFormLayout::DontWrapRows);

        sessionValue_ = makeTimeValue(content_card_);
        uptimeValue_  = makeTimeValue(content_card_);

        form->addRow(makeCaption(QStringLiteral("Сессия"), content_card_), sessionValue_);
        form->addRow(makeCaption(QStringLiteral("Аптайм"), content_card_), uptimeValue_);

        card->addLayout(form);
    }

    root->addWidget(content_card_);

    // Таймер обновления — 250мс достаточно, 75мс визуально смысла почти не даёт
    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this] { refreshAll(); });
    timer_.start();

    refreshAll();
}

QtAppStatusBarWidget::~QtAppStatusBarWidget() = default;

void QtAppStatusBarWidget::refreshAll()
{
    setState(vm_.state());
    setSession(vm_.sessionTime());
    setUptime(vm_.uptimeTime());
}

void QtAppStatusBarWidget::setState(domain::common::ProcessLifecycleState s)
{
    const auto txt = stateToText(s);
    const auto accent = stateAccent(s);

    statusBadge_->setText(txt);

    // Badge цвет: лёгкий фон + рамка + текст
    statusBadge_->setStyleSheet(QString(
        "QLabel {"
        " background: %1;"
        " border: 1px solid %2;"
        " color: %3;"
        "}"
    ).arg(
        accent + "22",   // фон (с альфой)
        accent + "55",   // border
        accent           // текст
    ));
}

void QtAppStatusBarWidget::setSession(domain::common::Timestamp ts)
{
    sessionValue_->setText(formatHhMmSs(ts));
}

void QtAppStatusBarWidget::setUptime(domain::common::Timestamp ts)
{
    uptimeValue_->setText(formatHhMmSs(ts));
}

QString QtAppStatusBarWidget::stateToText(domain::common::ProcessLifecycleState s)
{
    using S = domain::common::ProcessLifecycleState;
    switch (s) {
        case S::Idle:     return QStringLiteral("Ожидание");
        case S::Forward:  return QStringLiteral("Прямой ход");
        case S::Backward: return QStringLiteral("Обратный ход");
        case S::Stopping: return QStringLiteral("Остановка");
    }
    return QStringLiteral("Неизвестно");
}

QString QtAppStatusBarWidget::stateAccent(domain::common::ProcessLifecycleState s)
{
    using S = domain::common::ProcessLifecycleState;
    switch (s) {
        case S::Idle:     return "#6B7280"; // gray
        case S::Forward:  return "#059669"; // green
        case S::Backward: return "#2563EB"; // blue
        case S::Stopping: return "#D97706"; // amber
    }
    return "#111827";
}

QString QtAppStatusBarWidget::formatHhMmSs(domain::common::Timestamp ts)
{
    const auto d = ts.toDuration();

    using namespace std::chrono;
    const auto total = duration_cast<seconds>(d).count();

    const auto h = total / 3600;
    const auto m = (total % 3600) / 60;
    const auto s = total % 60;

    return QString("%1:%2:%3")
        .arg(qint64(h), 2, 10, QChar('0'))
        .arg(qint64(m), 2, 10, QChar('0'))
        .arg(qint64(s), 2, 10, QChar('0'));
}

} // namespace ui
