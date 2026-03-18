#ifndef CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
#define CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "viewmodels/Observable.h"
#include "viewmodels/calibration/result/CalibrationResultSaveViewModel.h"

namespace ui {

class QtCalibrationResultSaveWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtCalibrationResultSaveWidget(mvvm::CalibrationResultSaveViewModel& vm, QWidget* parent = nullptr);
    ~QtCalibrationResultSaveWidget() override;

private:
    void setupUi();
    void bind();
    void updateStateBadge(mvvm::CalibrationResultSaveState state, const QString& text);
    void openInExplorer(const std::filesystem::path& path);

    mvvm::CalibrationResultSaveViewModel& vm_;
    QLabel* batchLabel_{nullptr};
    QLabel* stateLabel_{nullptr};
    QLabel* errorLabel_{nullptr};
    QPushButton* saveButton_{nullptr};
    QPushButton* showInExplorerButton_{nullptr};
    QPushButton* saveAsButton_{nullptr};

    mvvm::Observable<std::string>::Subscription batchTextSub_;
    mvvm::Observable<mvvm::CalibrationResultSaveState>::Subscription saveStateSub_;
    mvvm::Observable<std::string>::Subscription saveStateTextSub_;
    mvvm::Observable<std::string>::Subscription errorTextSub_;
    mvvm::Observable<bool>::Subscription canSaveSub_;
    mvvm::Observable<bool>::Subscription canSaveAsSub_;
    mvvm::Observable<bool>::Subscription canShowInExplorerSub_;
};

} // namespace ui

#endif //CLEANGRADUATOR_QTCALIBRATIONRESULTSAVEWIDGET_H
