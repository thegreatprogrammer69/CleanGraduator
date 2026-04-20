#ifndef CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H


#include <QWidget>

#include <functional>
#include <thread>
#include <vector>

#include "viewmodels/Observable.h"

namespace mvvm {
    class CameraGridSettingsViewModel;
}

class QLineEdit;
class QPushButton;
class QCheckBox;
class QComboBox;

class QtCameraGridSettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtCameraGridSettingsWidget(
        mvvm::CameraGridSettingsViewModel& model,
        QWidget* parent = nullptr
    );

    ~QtCameraGridSettingsWidget() override;

    signals:
        void crosshairAppearanceRequested();

private:
    void buildUi();
    void connectUi();
    void connectViewModel();
    void setBusy(bool busy);
    void joinWorkerIfNeeded();
    void runCameraOperation(const std::function<std::vector<int>()>& operation);

private:
    mvvm::CameraGridSettingsViewModel& model_;
    mvvm::Observable<std::string>::Subscription cameraInputSub_;

    QLineEdit*   camerasEdit_{nullptr};
    QComboBox* cameraCountCombo_{nullptr};
    QPushButton* openButton_{nullptr};
    QPushButton* openAllButton_{nullptr};
    QPushButton* closeAllButton_{nullptr};

    bool busy_{false};
    std::thread worker_;
};


#endif //CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H
