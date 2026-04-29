#include "QtControlWidget.h"

#include <algorithm>
#include <QFrame>
#include <sstream>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMetaObject>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QRegularExpression>

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
    auto* layout = new QVBoxLayout(box);

    auto* textRow = new QHBoxLayout();
    auto* dot = new QLabel("●", box);
    dot->setStyleSheet("color:#22c55e; font-size: 20px;");
    statusTextLabel_ = new QLabel(tr("Ожидание запуска"), box);
    textRow->addWidget(dot);
    textRow->addWidget(statusTextLabel_, 1);

    forwardProgressBar_ = new QProgressBar(box);
    forwardProgressBar_->setRange(0, 100);
    forwardProgressBar_->setValue(0);
    forwardProgressBar_->setFormat(tr("Прямой ход: %p%"));
    forwardProgressBar_->setStyleSheet("QProgressBar::chunk { background-color: #3b82f6; }");

    backwardProgressBar_ = new QProgressBar(box);
    backwardProgressBar_->setRange(0, 100);
    backwardProgressBar_->setValue(0);
    backwardProgressBar_->setFormat(tr("Обратный ход: %p%"));
    backwardProgressBar_->setStyleSheet("QProgressBar::chunk { background-color: #ef4444; }");

    layout->addLayout(textRow);
    layout->addWidget(forwardProgressBar_);
    layout->addWidget(backwardProgressBar_);
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

    updateProgressBars(statusTextLabel_->text());
}

void QtControlWidget::bind()
{
    statusTextSub_ = vm_.calibrationViewModel().status_text.subscribe([this](const auto& change) {
        QMetaObject::invokeMethod(this, [this, text = QString::fromStdString(change.new_value)] {
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

    const auto initial_status = QString::fromStdString(vm_.calibrationViewModel().status_text.get_copy());
    statusTextLabel_->setText(initial_status);
    updateProgressBars(initial_status);
    moveForwardButton_->setEnabled(!vm_.motorViewModel().is_running.get_copy());
    moveBackwardButton_->setEnabled(!vm_.motorViewModel().is_running.get_copy());
    motorStopButton_->setEnabled(vm_.motorViewModel().is_running.get_copy());
    refreshMetrics();
}


void QtControlWidget::updateProgressBars(const QString& status_text)
{
    if (status_text.contains(QStringLiteral("Прямой ход"), Qt::CaseInsensitive)) {
        const int progress = parseForwardProgressPercent(status_text);
        if (progress >= 0) {
            forwardProgressBar_->setValue(progress);
        }

        const double target_pressure = parseForwardTargetPressure(status_text);
        if (target_pressure > 0.0) {
            lastForwardTargetPressure_ = target_pressure;
        }
        return;
    }

    if (status_text.contains(QStringLiteral("Обратный ход"), Qt::CaseInsensitive)) {
        backwardProgressBar_->setValue(calculateBackwardProgressPercent());
    }
}

int QtControlWidget::parseForwardProgressPercent(const QString& status_text) const
{
    static const QRegularExpression re(QStringLiteral(R"(\((\d+)\s*%\))"));
    const auto match = re.match(status_text);
    if (!match.hasMatch()) {
        return -1;
    }
    return std::clamp(match.captured(1).toInt(), 0, 100);
}

double QtControlWidget::parseForwardTargetPressure(const QString& status_text) const
{
    static const QRegularExpression re(QStringLiteral(R"(([-+]?\d+(?:[\.,]\d+)?)\s*/\s*([-+]?\d+(?:[\.,]\d+)?))"));
    const auto match = re.match(status_text);
    if (!match.hasMatch()) {
        return 0.0;
    }

    auto target_text = match.captured(2);
    target_text.replace(',', '.');
    return target_text.toDouble();
}

int QtControlWidget::calculateBackwardProgressPercent() const
{
    const auto pressure = vm_.pressureSensorViewModel().pressure.get_copy();
    const double current_pressure = pressure.pa();

    const double start_pressure = std::max(lastForwardTargetPressure_, 0.001);
    const double progress = std::clamp((start_pressure - current_pressure) / start_pressure, 0.0, 1.0);
    return static_cast<int>(progress * 100.0 + 0.5);
}

} // namespace ui
