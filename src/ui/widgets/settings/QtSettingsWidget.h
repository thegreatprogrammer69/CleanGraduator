#ifndef CLEANGRADUATOR_QTSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTSETTINGSWIDGET_H


#include <QWidget>


namespace mvvm {
    class CameraGridSettingsViewModel;
}

class QtCameraGridSettingsWidget;
class QGroupBox;

struct QtSettingsWidgetViewModels {
    mvvm::CameraGridSettingsViewModel& cameraGridSettings;
};

class QtSettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtSettingsWidget(
        QtSettingsWidgetViewModels models,
        QWidget* parent = nullptr
    );

    signals:
        void crosshairAppearanceRequested();

private:
    void buildUi();
    void connectUi();

private:
    QtSettingsWidgetViewModels models_;

    QtCameraGridSettingsWidget* cameraGridWidget_{nullptr};
};


#endif //CLEANGRADUATOR_QTSETTINGSWIDGET_H