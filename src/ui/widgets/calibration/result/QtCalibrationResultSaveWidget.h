#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H

#include <QWidget>

#include "viewmodels/calibration/result/CalibrationResultSaveViewModel.h"

class QLabel;
class QPushButton;

namespace ui {
    class QtCalibrationResultSaveWidget final : public QWidget {
        Q_OBJECT
    public:
        explicit QtCalibrationResultSaveWidget(mvvm::CalibrationResultSaveViewModel& vm,
                                               QWidget* parent = nullptr);
        ~QtCalibrationResultSaveWidget() override;

    private:
        void buildUi();
        void connectUi();
        void connectViewModel();
        void applyState(const mvvm::CalibrationResultSaveViewModel::ViewState& state);

    private:
        mvvm::CalibrationResultSaveViewModel& vm_;
        mvvm::Observable<mvvm::CalibrationResultSaveViewModel::ViewState>::Subscription state_sub_;
        QLabel* party_label_{nullptr};
        QLabel* status_label_{nullptr};
        QLabel* path_label_{nullptr};
        QLabel* error_label_{nullptr};
        QPushButton* save_button_{nullptr};
        QPushButton* show_in_explorer_button_{nullptr};
        QPushButton* save_as_button_{nullptr};
    };
}

#endif //CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
