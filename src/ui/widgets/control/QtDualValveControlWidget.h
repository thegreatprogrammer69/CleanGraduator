#ifndef CLEANGRADUATOR_QTDUALVALVECONTROLWIDGET_H
#define CLEANGRADUATOR_QTDUALVALVECONTROLWIDGET_H

#include <QWidget>

#include "viewmodels/Observable.h"
#include "viewmodels/control/DualValveControlViewModel.h"

class QPushButton;
class QLabel;

namespace mvvm {
    class DualValveControlViewModel;
}

namespace ui {

    class QtDualValveControlWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtDualValveControlWidget(mvvm::DualValveControlViewModel& vm, QWidget* parent = nullptr);
        ~QtDualValveControlWidget() override;

    private:
        void setupUi();
        void setupConnections();
        void bindViewModel();

        void updateUiFromState(mvvm::DualValveControlViewModel::FlapsState state);

    private:
        mvvm::DualValveControlViewModel& vm_;

        QPushButton* open_input_btn_{nullptr};
        QPushButton* open_output_btn_{nullptr};
        QPushButton* close_flaps_btn_{nullptr};
        QLabel* state_label_{nullptr};

        // подписка на Observable
        mvvm::Observable<mvvm::DualValveControlViewModel::FlapsState>::Subscription state_sub_;
    };

} // namespace ui

#endif //CLEANGRADUATOR_QTDUALVALVECONTROLWIDGET_H