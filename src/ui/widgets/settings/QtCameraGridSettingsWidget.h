#ifndef CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H


#include <QWidget>

#include "application/dto/settings/camera_grid/VideoSourceGridSettings.h"
#include "viewmodels/Observable.h"

namespace mvvm {
    class VideoSourceGridSettingsViewModel;
}

class QLineEdit;
class QPushButton;
class QCheckBox;

class QtCameraGridSettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtCameraGridSettingsWidget(
        mvvm::VideoSourceGridSettingsViewModel& model,
        QWidget* parent = nullptr
    );

    ~QtCameraGridSettingsWidget() override;

    signals:
        void crosshairAppearanceRequested();

private:
    void buildUi();
    void connectUi();
    void connectViewModel();

private:
    mvvm::VideoSourceGridSettingsViewModel& model_;

    mvvm::Observable<application::dto::VideoSourceGridSettings>::Subscription settingsSub_;
    mvvm::Observable<std::string>::Subscription errorSub_;

    QLineEdit*   camerasEdit_{nullptr};
    QPushButton* openButton_{nullptr};
    QPushButton* openAllButton_{nullptr};

    QCheckBox* autoOpenCheckBox_{nullptr};
    QCheckBox* drawCrosshairCheckBox_{nullptr};
    QPushButton* crosshairAppearanceButton_{nullptr};
};


#endif //CLEANGRADUATOR_QTCAMERASETTINGSWIDGET_H