// ui/widgets/QtAppStatusBarWidget.h
#ifndef CLEANGRADUATOR_QTAPPSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTAPPSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include "domain/core/calibration/lifecycle/CalibrationLifecycleState.h"
#include "domain/core/measurement/Timestamp.h"

namespace mvvm { class AppStatusBarViewModel; }

class QFrame;
class QLabel;

namespace ui {

    class QtAppStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtAppStatusBarWidget(mvvm::AppStatusBarViewModel& vm, QWidget* parent = nullptr);
        ~QtAppStatusBarWidget() override;

    private:
        static QString stateToText(domain::common::CalibrationLifecycleState s);
        static QString formatHhMmSs(domain::common::Timestamp ts);
        static QString stateAccent(domain::common::CalibrationLifecycleState s);

        void refreshAll();

        void setState(domain::common::CalibrationLifecycleState s);
        void setSession(domain::common::Timestamp ts);
        void setUptime(domain::common::Timestamp ts);

    private:
        mvvm::AppStatusBarViewModel& vm_;

        QFrame* content_card_{nullptr};

        QLabel* statusBadge_{nullptr};
        QLabel* sessionValue_{nullptr};
        QLabel* uptimeValue_{nullptr};

        QTimer timer_;
    };

} // namespace ui

#endif // CLEANGRADUATOR_QTAPPSTATUSBARWIDGET_H
