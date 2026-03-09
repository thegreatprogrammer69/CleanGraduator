#ifndef CLEANGRADUATOR_QTAPPSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTAPPSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include "application/orchestrators/calibration/process/CalibrationOrchestratorState.h"
#include "domain/core/measurement/Timestamp.h"

namespace mvvm { class AppStatusBarViewModel; }

class QLabel;

namespace ui {

    class QtAppStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtAppStatusBarWidget(mvvm::AppStatusBarViewModel& vm, QWidget* parent = nullptr);
        ~QtAppStatusBarWidget() override;

    private:
        static QString stateToText(application::orchestrators::CalibrationOrchestratorState s);
        static QString formatHhMmSs(domain::common::Timestamp ts);

        void refreshAll();

        void setState(application::orchestrators::CalibrationOrchestratorState s);
        void setSession(domain::common::Timestamp ts);
        void setUptime(domain::common::Timestamp ts);

    private:
        mvvm::AppStatusBarViewModel& vm_;

        QLabel* stateValue_{nullptr};
        QLabel* sessionValue_{nullptr};
        QLabel* uptimeValue_{nullptr};

        QTimer timer_;
    };

}

#endif