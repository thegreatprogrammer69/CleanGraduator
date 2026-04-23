#include "QtProcessStatusWidget.h"

#include <QHBoxLayout>
#include <QLabel>

#include <chrono>
#include <sstream>

#include "viewmodels/control/CalibrationSessionControlViewModel.h"
#include "viewmodels/status_bar/AppStatusBarViewModel.h"
#include "viewmodels/status_bar/PressureSensorStatusBarViewModel.h"

namespace ui {

QtProcessStatusWidget::QtProcessStatusWidget(
    mvvm::CalibrationSessionControlViewModel& calibration_vm,
    mvvm::AppStatusBarViewModel& app_status_vm,
    mvvm::PressureSensorStatusBarViewModel& pressure_vm,
    QWidget* parent)
    : QWidget(parent)
    , calibration_vm_(calibration_vm)
    , app_status_vm_(app_status_vm)
    , pressure_vm_(pressure_vm)
{
    auto* layout = new QVBoxLayout(this);

    statusLabel_ = new QLabel(this);
    timeLabel_ = new QLabel(this);
    pressureLabel_ = new QLabel(this);
    speedLabel_ = new QLabel(this);

    layout->addWidget(new QLabel(tr("Статус"), this));
    layout->addWidget(statusLabel_);

    auto* metrics = new QHBoxLayout();
    metrics->addWidget(timeLabel_);
    metrics->addWidget(pressureLabel_);
    metrics->addWidget(speedLabel_);
    metrics->addStretch();

    layout->addLayout(metrics);

    status_sub_ = calibration_vm_.status_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)] {
            statusLabel_->setText(text);
        }, Qt::QueuedConnection);
    }, false);

    timer_.setInterval(200);
    connect(&timer_, &QTimer::timeout, this, [this] { refresh(); });
    timer_.start();
    refresh();
}

QtProcessStatusWidget::~QtProcessStatusWidget() = default;

QString QtProcessStatusWidget::formatMmSs(domain::common::Timestamp ts)
{
    using namespace std::chrono;
    const auto total = duration_cast<seconds>(ts.toDuration()).count();
    const auto m = total / 60;
    const auto s = total % 60;
    return QString("%1:%2 мин.").arg(qint64(m), 2, 10, QChar('0')).arg(qint64(s), 2, 10, QChar('0'));
}

void QtProcessStatusWidget::refresh()
{
    timeLabel_->setText(tr("Время: %1").arg(formatMmSs(app_status_vm_.sessionTime())));

    std::stringstream pressure_ss;
    pressure_ss << pressure_vm_.pressure.get_copy();
    pressureLabel_->setText(tr("Давление: %1").arg(QString::fromStdString(pressure_ss.str())));

    std::stringstream speed_ss;
    speed_ss << pressure_vm_.pressureSpeedPerSecond();
    speedLabel_->setText(tr("Скорость роста двл.: %1").arg(QString::fromStdString(speed_ss.str())));
}

} // namespace ui
