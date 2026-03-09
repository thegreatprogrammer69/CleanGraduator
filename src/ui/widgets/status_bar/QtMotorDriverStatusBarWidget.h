#ifndef CLEANGRADUATOR_QTMOTORDRIVERSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTMOTORDRIVERSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include <memory>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/core/drivers/motor/MotorLimitsState.h"

namespace mvvm { class MotorDriverStatusViewModel; }

class QLabel;

namespace ui {

    class QtMotorDriverStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtMotorDriverStatusBarWidget(
            mvvm::MotorDriverStatusViewModel& vm,
            QWidget* parent = nullptr
        );

        ~QtMotorDriverStatusBarWidget() override;

    private:
        static QString directionArrow(domain::common::MotorDirection direction);

        void initializeFromViewModel();

        void refreshFrequency();
        void setRunning(bool running);
        void setDirection(domain::common::MotorDirection direction);
        void setLimits(domain::common::MotorLimitsState limits);

    private:
        mvvm::MotorDriverStatusViewModel& vm_;

        QLabel* running_value_{nullptr};
        QLabel* motion_value_{nullptr};

        QLabel* home_value_{nullptr};
        QLabel* end_value_{nullptr};

        struct Subscriptions;
        std::unique_ptr<Subscriptions> subs_;

        QTimer timer_;
    };

}

#endif