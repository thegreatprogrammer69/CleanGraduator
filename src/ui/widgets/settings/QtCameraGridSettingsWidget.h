#ifndef CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H


#include <QWidget>

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

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
    void enqueueCameraAction(std::function<void()> action);
    void cameraWorkerLoop();

private:
    mvvm::CameraGridSettingsViewModel& model_;
    mvvm::Observable<std::string>::Subscription cameraInputSub_;

    QLineEdit*   camerasEdit_{nullptr};
    QComboBox* cameraCountCombo_{nullptr};
    QPushButton* openButton_{nullptr};
    QPushButton* openAllButton_{nullptr};
    QPushButton* closeAllButton_{nullptr};

    std::thread cameraWorker_;
    std::mutex cameraActionsMutex_;
    std::condition_variable cameraActionsCv_;
    std::deque<std::function<void()>> cameraActions_;
    bool stopCameraWorker_{false};
};


#endif //CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H
