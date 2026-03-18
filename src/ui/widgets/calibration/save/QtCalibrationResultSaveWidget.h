#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "viewmodels/Observable.h"
#include "viewmodels/calibration/save/CalibrationResultSaveViewModel.h"

namespace ui {

class QtCalibrationResultSaveWidget final : public QWidget {
    Q_OBJECT
public:
    explicit QtCalibrationResultSaveWidget(mvvm::CalibrationResultSaveViewModel& vm, QWidget* parent = nullptr);
    ~QtCalibrationResultSaveWidget() override;

private:
    void setupUi();
    void bind();
    void applyState(mvvm::CalibrationResultSaveViewModel::SaveState state);

    mvvm::CalibrationResultSaveViewModel& vm_;
    QLabel* partyLabel_{nullptr};
    QLabel* statusLabel_{nullptr};
    QLabel* errorLabel_{nullptr};
    QPushButton* saveButton_{nullptr};
    QPushButton* showInExplorerButton_{nullptr};
    QPushButton* saveAsButton_{nullptr};

    mvvm::Observable<int>::Subscription partySub_;
    mvvm::Observable<std::string>::Subscription statusSub_;
    mvvm::Observable<mvvm::CalibrationResultSaveViewModel::SaveState>::Subscription stateSub_;
    mvvm::Observable<std::string>::Subscription errorSub_;
    mvvm::Observable<bool>::Subscription canSaveSub_;
    mvvm::Observable<bool>::Subscription canShowSub_;
    mvvm::Observable<bool>::Subscription canSaveAsSub_;
};

}

#endif //CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
