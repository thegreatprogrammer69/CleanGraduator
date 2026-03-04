#ifndef CLEANGRADUATOR_QTMOTORDRIVERSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTMOTORDRIVERSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include <memory>

#include "domain/core/drivers/motor/MotorDirection.h"
#include "domain/core/drivers/motor/MotorDriverError.h"
#include "domain/core/drivers/motor/MotorLimitsState.h"

namespace mvvm { class MotorDriverStatusViewModel; }

class QFrame;
class QLabel;

namespace ui {

    class QtMotorDriverStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtMotorDriverStatusBarWidget(mvvm::MotorDriverStatusViewModel& vm, QWidget* parent = nullptr);
        ~QtMotorDriverStatusBarWidget() override;

    private:
        static QString directionToText(domain::common::MotorDirection direction);
        static QString runToText(bool is_running);
        static QString boolToText(bool value);

        static QString errorToText(const std::string& error);

        // --- Initial sync ---
        void initializeFromViewModel();

        void refreshFrequency();
        void setRunning(bool is_running);
        void setDirection(domain::common::MotorDirection direction);
        void setLimits(domain::common::MotorLimitsState limits);
        void setError(const std::string& error);

    private:
        mvvm::MotorDriverStatusViewModel& vm_;

        QFrame* content_card_{nullptr};

        QLabel* run_value_{nullptr};
        QLabel* freq_value_{nullptr};
        QLabel* direction_value_{nullptr};
        QLabel* home_limit_value_{nullptr};
        QLabel* end_limit_value_{nullptr};
        QLabel* fault_value_{nullptr};

        struct Subscriptions;
        std::unique_ptr<Subscriptions> subs_;

        QTimer timer_;
    };

} // namespace ui

#endif // CLEANGRADUATOR_QTMOTORDRIVERSTATUSBARWIDGET_H