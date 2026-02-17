#ifndef CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H
#define CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H

#include <QWidget>
#include <QTimer>

#include <memory>

namespace mvvm { class PressureSensorStatusBarViewModel; }

class QFrame;
class QLabel;
class QPushButton;

namespace ui {

    class QtPressureSensorStatusBarWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtPressureSensorStatusBarWidget(mvvm::PressureSensorStatusBarViewModel& vm, QWidget* parent = nullptr);
        ~QtPressureSensorStatusBarWidget() override;

    private:
        static QString openedToText(bool is_opened);
        static QString actionToText(bool is_opened);
        static QString errorToText(const std::string& err);

        void refreshAll();
        void setOpenedText(bool is_opened);
        void setActionButtonText(bool is_opened);
        void setPressureText();
        void setErrorText(const std::string& err);

    private:
        mvvm::PressureSensorStatusBarViewModel& vm_;

        QFrame* content_card_{nullptr};

        QLabel* opened_value_{nullptr};
        QLabel* pressure_value_{nullptr};
        QLabel* speed_value_{nullptr};
        QLabel* error_value_{nullptr};
        QPushButton* action_button_{nullptr};

        struct Subscriptions;
        std::unique_ptr<Subscriptions> subs_;

        QTimer timer_;
    };

} // namespace ui

#endif //CLEANGRADUATOR_QTPRESSURESENSORSTATUSBARWIDGET_H
