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
    explicit QtCalibrationResultSaveWidget(
        mvvm::CalibrationResultSaveViewModel& vm,
        QWidget* parent = nullptr);

private:
    void setupUi();
    void bindViewModel();
    void polishButtonStyles();
    void applyStatus(mvvm::CalibrationResultSaveViewModel::Status status, const QString& text);

private:
    mvvm::CalibrationResultSaveViewModel& vm_;

    QLabel* batch_label_ = nullptr;
    QLabel* status_badge_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* save_button_ = nullptr;
    QPushButton* reveal_button_ = nullptr;
    QPushButton* save_as_button_ = nullptr;

    mvvm::Observable<std::optional<int>>::Subscription batch_number_sub_;
    mvvm::Observable<mvvm::CalibrationResultSaveViewModel::Status>::Subscription status_sub_;
    mvvm::Observable<std::string>::Subscription status_text_sub_;
    mvvm::Observable<std::string>::Subscription error_text_sub_;
    mvvm::Observable<bool>::Subscription can_save_sub_;
    mvvm::Observable<bool>::Subscription can_reveal_sub_;
};

}

#endif //CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
