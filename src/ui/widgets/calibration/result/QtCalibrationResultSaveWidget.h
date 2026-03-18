#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H

#include <memory>

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
    struct Subscriptions;

    void setupUi();
    void bindViewModel();
    void chooseSaveAsDirectory();
    void openInExplorer();

    mvvm::CalibrationResultSaveViewModel& vm_;
    QLabel* party_label_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* error_label_{nullptr};
    QPushButton* save_button_{nullptr};
    QPushButton* show_in_explorer_button_{nullptr};
    QPushButton* save_as_button_{nullptr};
    std::unique_ptr<Subscriptions> subscriptions_;
};

}

#endif //CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
