#include "QtAppStatusBarWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QFont>

#include <chrono>

#include "viewmodels/status_bar/AppStatusBarViewModel.h"

namespace ui {

static QLabel* makeSeparator(QWidget* parent)
{
    auto* l = new QLabel("|", parent);
    return l;
}

static QLabel* makeTime(QWidget* parent)
{
    auto* l = new QLabel(parent);

    QFont f = l->font();
    f.setFamily("Consolas");
    l->setFont(f);

    return l;
}

    QtAppStatusBarWidget::QtAppStatusBarWidget(mvvm::AppStatusBarViewModel& vm, QWidget* parent)
        : QWidget(parent)
        , vm_(vm)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4,0,4,0);
    layout->setSpacing(6);

    stateValue_ = new QLabel(this);

    uptimeValue_ = makeTime(this);
    sessionValue_ = makeTime(this);

    layout->addWidget(stateValue_);

    layout->addWidget(makeSeparator(this));

    layout->addWidget(uptimeValue_);

    layout->addWidget(makeSeparator(this));

    layout->addWidget(sessionValue_);

    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this]{ refreshAll(); });
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

void QtAppStatusBarWidget::setState(application::orchestrators::CalibrationOrchestratorState s)
{
    stateValue_->setText(stateToText(s));
}

void QtAppStatusBarWidget::setSession(domain::common::Timestamp ts)
{
    sessionValue_->setText(formatHhMmSs(ts));
}

void QtAppStatusBarWidget::setUptime(domain::common::Timestamp ts)
{
    uptimeValue_->setText(formatHhMmSs(ts));
}

QString QtAppStatusBarWidget::stateToText(application::orchestrators::CalibrationOrchestratorState s)
{
    using S = application::orchestrators::CalibrationOrchestratorState;

    switch (s) {
        case S::Stopped:  return QObject::tr("Ожидание");
        case S::Starting: return QObject::tr("Запуск");
        case S::Started:  return QObject::tr("В процессе");
        case S::Stopping: return QObject::tr("Остановка");
    }

    return QObject::tr("Неизвестно");
}

QString QtAppStatusBarWidget::formatHhMmSs(domain::common::Timestamp ts)
{
    using namespace std::chrono;

    const auto total =
        duration_cast<seconds>(ts.toDuration()).count();

    const auto h = total / 3600;
    const auto m = (total % 3600) / 60;
    const auto s = total % 60;

    return QString("%1:%2:%3")
        .arg(qint64(h),2,10,QChar('0'))
        .arg(qint64(m),2,10,QChar('0'))
        .arg(qint64(s),2,10,QChar('0'));
}

}