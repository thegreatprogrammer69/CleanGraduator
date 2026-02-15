#ifndef CLEANGRADUATOR_QTSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTSETTINGSWIDGET_H


#include <QWidget>


namespace mvvm {
    class SettingsViewModel;
}

class QtCameraGridSettingsWidget;
class QGroupBox;

class QtSettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtSettingsWidget(
        mvvm::SettingsViewModel& model,
        QWidget* parent = nullptr
    );

    signals:
        void crosshairAppearanceRequested();

private:
    void buildUi();
    void connectUi();

private:
    mvvm::SettingsViewModel& model_;

    QtCameraGridSettingsWidget* cameraGridWidget_{nullptr};
};


#endif //CLEANGRADUATOR_QTSETTINGSWIDGET_H