#include "QtAppStatusBarWidget.h"

#include <QHBoxLayout>
#include <QLabel>

#include "viewmodels/status_bar/AppStatusBarViewModel.h"

namespace ui {

QtAppStatusBarWidget::QtAppStatusBarWidget(mvvm::AppStatusBarViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4,0,4,0);
    layout->setSpacing(6);

    stateValue_ = new QLabel(this);
    layout->addWidget(stateValue_);

    timer_.setInterval(250);
    connect(&timer_, &QTimer::timeout, this, [this]{ refreshAll(); });
    timer_.start();

    refreshAll();
}

QtAppStatusBarWidget::~QtAppStatusBarWidget() = default;

void QtAppStatusBarWidget::refreshAll()
{
    setState(vm_.state());
}

void QtAppStatusBarWidget::setState(application::orchestrators::CalibrationOrchestratorState s)
{
    stateValue_->setText(stateToText(s));
}

void QtAppStatusBarWidget::setSession(domain::common::Timestamp) {}
void QtAppStatusBarWidget::setUptime(domain::common::Timestamp) {}

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

QString QtAppStatusBarWidget::formatHhMmSs(domain::common::Timestamp)
{
    return {};
}

} // namespace ui
