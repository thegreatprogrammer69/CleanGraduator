#ifndef CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include <memory>

namespace mvvm { class PressureSensorStatusBarViewModel; }

class QLabel;

namespace ui {

    class QtPressureSensorStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtPressureSensorStatusBarWidget(
            mvvm::PressureSensorStatusBarViewModel& vm,
            QWidget* parent = nullptr
        );

        ~QtPressureSensorStatusBarWidget() override;

    private:
        static QString openedToText(bool opened);

        void refreshAll();
        void setOpened(bool opened);

    private:
        mvvm::PressureSensorStatusBarViewModel& vm_;

        QLabel* opened_value_{nullptr};

        struct Subscriptions;
        std::unique_ptr<Subscriptions> subs_;

        QTimer timer_;
    };

}

#endif