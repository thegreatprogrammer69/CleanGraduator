#include "QtControlWidget.h"

#include <QFrame>
#include <algorithm>
#include <sstream>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMetaObject>
#include <QProgressBar>
#include <QRegularExpression>
#include <QVBoxLayout>

#include "QtCalibrationSessionControlWidget.h"
#include "ui/widgets/calibration/result/QtCalibrationResultSaveWidget.h"
#include "viewmodels/control/CalibrationSessionControlViewModel.h"
#include "viewmodels/control/ControlViewModel.h"
#include "viewmodels/control/DualValveControlViewModel.h"
#include "viewmodels/control/MotorControlViewModel.h"
#include "viewmodels/status_bar/AppStatusBarViewModel.h"
#include "viewmodels/status_bar/PressureSensorStatusBarViewModel.h"

namespace {
constexpr int kManualMotorFrequency = 2000;
}

namespace ui {

QtControlWidget::QtControlWidget(mvvm::ControlViewModel& vm, QWidget* parent)
    : QWidget(parent)
    , vm_(vm)
{
    setupUi();
    bind();
}

void QtControlWidget::setupUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* top = new QHBoxLayout();
    top->addWidget(makeCalibrationSection(), 1);
    top->addWidget(makeResultSection(), 1);
    top->addWidget(makeManualSection(), 1);
    root->addLayout(top);

    root->addWidget(makeStatusSection());
    root->addWidget(makeMetricsSection());
}

QWidget* QtControlWidget::makeCalibrationSection()
{
    auto* box = new QGroupBox(tr("Градуировка"), this);
    auto* layout = new QVBoxLayout(box);
    layout->addWidget(new QtCalibrationSessionControlWidget(vm_.calibrationViewModel(), box));
    return box;
}

QWidget* QtControlWidget::makeResultSection()
{
    auto* box = new QGroupBox(tr("Результат"), this);
    auto* layout = new QVBoxLayout(box);
    layout->addWidget(new QtCalibrationResultSaveWidget(vm_.calibrationResultSaveViewModel(), box));
    return box;
}

QWidget* QtControlWidget::makeManualSection()
{
    auto* box = new QGroupBox(tr("Ручное управление"), this);
    auto* layout = new QGridLayout(box);

    moveForwardButton_ = new QPushButton(tr("Дв. вперёд"), box);
    motorStopButton_ = new QPushButton(tr("Стоп двигатель"), box);
    moveBackwardButton_ = new QPushButton(tr("Дв. назад"), box);

    auto* closeFlaps = new QPushButton(tr("Закрыть клапана"), box);
    auto* openIntake = new QPushButton(tr("Открыть впускной"), box);
    auto* openExhaust = new QPushButton(tr("Открыть выпускной"), box);

    layout->addWidget(moveForwardButton_, 0, 0);
    layout->addWidget(motorStopButton_, 0, 1);
    layout->addWidget(moveBackwardButton_, 0, 2);
    layout->addWidget(closeFlaps, 1, 0, 1, 3);
    layout->addWidget(openIntake, 2, 0, 1, 2);
    layout->addWidget(openExhaust, 2, 2);

    connect(moveForwardButton_, &QPushButton::clicked, this, [this] {
        auto& vm = vm_.motorViewModel();
        vm.setDirection(domain::common::MotorDirection::Forward);
        vm.setFrequency(kManualMotorFrequency);
        vm.start();
    });
    connect(moveBackwardButton_, &QPushButton::clicked, this, [this] {
        auto& vm = vm_.motorViewModel();
        vm.setDirection(domain::common::MotorDirection::Backward);
        vm.setFrequency(kManualMotorFrequency);
        vm.start();
    });
    connect(motorStopButton_, &QPushButton::clicked, this, [this] {
        vm_.motorViewModel().stop();
    });

    connect(closeFlaps, &QPushButton::clicked, this, [this] { vm_.valvesViewModel().closeFlaps(); });
    connect(openIntake, &QPushButton::clicked, this, [this] { vm_.valvesViewModel().openInputFlap(); });
    connect(openExhaust, &QPushButton::clicked, this, [this] { vm_.valvesViewModel().openOutputFlap(); });

    return box;
}

QWidget* QtControlWidget::makeStatusSection()
{
    auto* box = new QGroupBox(tr("Статус"), this);
    auto* rootLayout = new QVBoxLayout(box);

    auto* statusLayout = new QHBoxLayout();
    auto* dot = new QLabel("●", box);
    dot->setStyleSheet("color:#22c55e; font-size: 20px;");
    statusTextLabel_ = new QLabel(tr("Ожидание запуска"), box);
    statusLayout->addWidget(dot);
    statusLayout->addWidget(statusTextLabel_, 1);

    forwardPressureProgressBar_ = new QProgressBar(box);
    forwardPressureProgressBar_->setRange(0, 100);
    forwardPressureProgressBar_->setFormat(tr("Прямой ход: %p%"));
    forwardPressureProgressBar_->setStyleSheet("QProgressBar::chunk { background-color: #2563eb; }");

    backwardPressureProgressBar_ = new QProgressBar(box);
    backwardPressureProgressBar_->setRange(0, 100);
    backwardPressureProgressBar_->setFormat(tr("Обратный ход: %p%"));
    backwardPressureProgressBar_->setStyleSheet("QProgressBar::chunk { background-color: #dc2626; }");

    rootLayout->addLayout(statusLayout);
    rootLayout->addWidget(forwardPressureProgressBar_);
    rootLayout->addWidget(backwardPressureProgressBar_);
    return box;
}

QWidget* QtControlWidget::makeMetricsSection()
{
    auto* w = new QWidget(this);
    auto* layout = new QGridLayout(w);

    sessionTimeLabel_ = new QLabel("00:00 мин.", w);
    pressureLabel_ = new QLabel("0", w);
    speedLabel_ = new QLabel("0", w);

    layout->addWidget(new QLabel(tr("Время:"), w), 0, 0);
    layout->addWidget(sessionTimeLabel_, 0, 1);
    layout->addWidget(new QLabel(tr("Давление:"), w), 1, 0);
    layout->addWidget(pressureLabel_, 1, 1);
    layout->addWidget(new QLabel(tr("Скорость роста двл.:"), w), 1, 2);
    layout->addWidget(speedLabel_, 1, 3);
    return w;
}

QString QtControlWidget::formatTime(domain::common::Timestamp ts) const
{
    using namespace std::chrono;
    const auto total_seconds = duration_cast<seconds>(ts.toDuration()).count();
    const auto minutes = total_seconds / 60;
    const auto seconds = total_seconds % 60;
    return QString("%1:%2 мин.")
        .arg(qint64(minutes), 2, 10, QChar('0'))
        .arg(qint64(seconds), 2, 10, QChar('0'));
}

void QtControlWidget::refreshMetrics()
{
    sessionTimeLabel_->setText(formatTime(vm_.appStatusViewModel().sessionTime()));

    std::stringstream pressure_stream;
    pressure_stream << vm_.pressureSensorViewModel().pressure.get_copy();
    pressureLabel_->setText(QString::fromStdString(pressure_stream.str()));

    std::stringstream speed_stream;
    speed_stream << vm_.pressureSensorViewModel().pressureSpeedPerSecond();
    speedLabel_->setText(QString::fromStdString(speed_stream.str()));

    const QString status_text = statusTextLabel_ ? statusTextLabel_->text() : QString();
    const auto pressure = vm_.pressureSensorViewModel().pressure.get_copy();
    if (status_text.startsWith(QStringLiteral("Прямой ход:"))) {
        const QRegularExpression re(QStringLiteral(R"(([-+]?\d*\.?\d+)\s*/\s*([-+]?\d*\.?\d+))"));
        const auto match = re.match(status_text);
        if (match.hasMatch()) {
            forwardTargetPressure_ = match.captured(2).toDouble();
        }
        const double safe_target = std::max(0.001, forwardTargetPressure_);
        const int forward_percent = static_cast<int>(std::clamp((pressure / safe_target) * 100.0, 0.0, 100.0));
        forwardPressureProgressBar_->setValue(forward_percent);
        backwardPressureProgressBar_->setValue(0);
    } else if (status_text.startsWith(QStringLiteral("Обратный ход:"))) {
        if (backwardStartPressure_ <= 0.0) {
            backwardStartPressure_ = pressure;
        }
        const double safe_start = std::max(0.001, backwardStartPressure_);
        const int backward_percent = static_cast<int>(std::clamp(((safe_start - pressure) / safe_start) * 100.0, 0.0, 100.0));
        backwardPressureProgressBar_->setValue(backward_percent);
    } else {
        backwardStartPressure_ = 0.0;
        forwardPressureProgressBar_->setValue(0);
        backwardPressureProgressBar_->setValue(0);
    }
}


void QtControlWidget::bind()
{
    const auto updateProgressBars = [this](const QString& status_text) {
        const auto pressure = vm_.pressureSensorViewModel().pressure.get_copy();
        if (status_text.startsWith(QStringLiteral("Прямой ход:"))) {
            const QRegularExpression re(QStringLiteral(R"(([-+]?\d*\.?\d+)\s*/\s*([-+]?\d*\.?\d+))"));
            const auto match = re.match(status_text);
            if (match.hasMatch()) {
                forwardTargetPressure_ = match.captured(2).toDouble();
            }

            const double safe_target = std::max(0.001, forwardTargetPressure_);
            const int forward_percent = static_cast<int>(std::clamp((pressure / safe_target) * 100.0, 0.0, 100.0));
            forwardPressureProgressBar_->setValue(forward_percent);
            backwardPressureProgressBar_->setValue(0);
        } else if (status_text.startsWith(QStringLiteral("Обратный ход:"))) {
            if (backwardStartPressure_ <= 0.0) {
                backwardStartPressure_ = pressure;
            }
            const double safe_start = std::max(0.001, backwardStartPressure_);
            const int backward_percent = static_cast<int>(std::clamp(((safe_start - pressure) / safe_start) * 100.0, 0.0, 100.0));
            backwardPressureProgressBar_->setValue(backward_percent);
        } else {
            backwardStartPressure_ = 0.0;
            if (!status_text.startsWith(QStringLiteral("Прямой ход:"))) {
                forwardPressureProgressBar_->setValue(0);
                backwardPressureProgressBar_->setValue(0);
            }
        }
    };

    statusTextSub_ = vm_.calibrationViewModel().status_text.subscribe([this, updateProgressBars](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value), updateProgressBars] {
            statusTextLabel_->setText(text);
            updateProgressBars(text);
        }, Qt::QueuedConnection);
    }, false);

    motorRunningSub_ = vm_.motorViewModel().is_running.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, running = change.new_value] {
            moveForwardButton_->setEnabled(!running);
            moveBackwardButton_->setEnabled(!running);
            motorStopButton_->setEnabled(running);
        }, Qt::QueuedConnection);
    }, false);

    metricsTimer_.setInterval(150);
    connect(&metricsTimer_, &QTimer::timeout, this, [this] { refreshMetrics(); });
    metricsTimer_.start();

    const QString initialStatusText = QString::fromStdString(vm_.calibrationViewModel().status_text.get_copy());
    statusTextLabel_->setText(initialStatusText);
    moveForwardButton_->setEnabled(!vm_.motorViewModel().is_running.get_copy());
    moveBackwardButton_->setEnabled(!vm_.motorViewModel().is_running.get_copy());
    motorStopButton_->setEnabled(vm_.motorViewModel().is_running.get_copy());
    forwardPressureProgressBar_->setValue(0);
    backwardPressureProgressBar_->setValue(0);
    refreshMetrics();
}

} // namespace ui
