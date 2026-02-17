#ifndef CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include <memory>

#include "domain/core/measurement/Pressure.h"

namespace mvvm {
    class PressureSensorStatusBarViewModel;
}

class QFrame;
class QLabel;

namespace ui {

    class QtPressureSensorStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtPressureSensorStatusBarWidget(mvvm::PressureSensorStatusBarViewModel& vm, QWidget* parent = nullptr);
        ~QtPressureSensorStatusBarWidget() override;

    private:
        static QString boolToText(bool value);
        static QString pressureToText(const domain::common::Pressure& value);
        static QString pressureSpeedToText(double pa_per_sec);

        void refreshAll();
        void setOpened(bool opened);
        void setPressure(const domain::common::Pressure& value);
        void setError(const std::string& error_text);

    private:
        mvvm::PressureSensorStatusBarViewModel& vm_;

        QFrame* content_card_{nullptr};

        QLabel* opened_value_{nullptr};
        QLabel* pressure_value_{nullptr};
        QLabel* pressure_speed_value_{nullptr};
        QLabel* error_value_{nullptr};

        struct Subscriptions;
        std::unique_ptr<Subscriptions> subs_;

        QTimer timer_;
    };

}

#endif //CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H
