#ifndef CLEANGRADUATOR_QTCONTROLWIDGET_H
#define CLEANGRADUATOR_QTCONTROLWIDGET_H

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

#include "domain/core/measurement/Timestamp.h"
#include "viewmodels/Observable.h"

namespace mvvm {
    class ControlViewModel;
}

namespace ui {

    class QtControlWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QtControlWidget(
            mvvm::ControlViewModel& vm,
            QWidget* parent = nullptr);

    private:
        void setupUi();
        void bind();
        QWidget* makeResultSection();
        QWidget* makeCalibrationSection();
        QWidget* makeManualSection();
        QWidget* makeStatusSection();
        QWidget* makeMetricsSection();

        QString formatTime(domain::common::Timestamp ts) const;
        void refreshMetrics();
        void refreshProgressBars(const std::string& status_text);

    private:
        mvvm::ControlViewModel& vm_;

        QLabel* statusTextLabel_{nullptr};
        QLabel* sessionTimeLabel_{nullptr};
        QLabel* pressureLabel_{nullptr};
        QLabel* speedLabel_{nullptr};
        QProgressBar* forwardProgressBar_{nullptr};
        QProgressBar* backwardProgressBar_{nullptr};
        double backwardStartPressure_{0.0};

        QPushButton* moveForwardButton_{nullptr};
        QPushButton* motorStopButton_{nullptr};
        QPushButton* moveBackwardButton_{nullptr};

        mvvm::Observable<bool>::Subscription motorRunningSub_;
        mvvm::Observable<std::string>::Subscription statusTextSub_;
        QTimer metricsTimer_;
    };

} // namespace ui

#endif
