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
    ~QtCalibrationResultSaveWidget() override;

private:
    void setupUi();
    void bind();
    void updateStatusPresentation(mvvm::CalibrationResultSaveViewModel::SaveState state);

    mvvm::CalibrationResultSaveViewModel& vm_;

    QLabel* partyLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* errorLabel_ = nullptr;
    QPushButton* saveButton_ = nullptr;
    QPushButton* showInExplorerButton_ = nullptr;
    QPushButton* saveAsButton_ = nullptr;

    mvvm::Subscription partySub_;
    mvvm::Subscription statusTextSub_;
    mvvm::Subscription statusStateSub_;
    mvvm::Subscription errorSub_;
    mvvm::Subscription canSaveSub_;
    mvvm::Subscription canShowInExplorerSub_;
    mvvm::Subscription canSaveAsSub_;
    mvvm::Subscription savedDirectorySub_;
};

} // namespace ui

#endif //CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
