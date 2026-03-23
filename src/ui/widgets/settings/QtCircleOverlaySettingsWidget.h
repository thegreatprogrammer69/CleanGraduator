#ifndef CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H

#include <QWidget>

#include <cstdint>

#include "viewmodels/Observable.h"

namespace mvvm {
class ICircleOverlaySettingsViewModel;
}

class QLabel;
class QPushButton;
class QSlider;

class QtCircleOverlaySettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtCircleOverlaySettingsWidget(
        mvvm::ICircleOverlaySettingsViewModel& model,
        QWidget* parent = nullptr
    );

    ~QtCircleOverlaySettingsWidget() override;

private:
    void buildUi();
    void connectUi();
    void connectViewModel();
    void applyColorButtonStyle(QPushButton* button, std::uint32_t color);

private:
    mvvm::ICircleOverlaySettingsViewModel& model_;

    mvvm::Observable<int>::Subscription diameterSub_;
    mvvm::Observable<std::uint32_t>::Subscription color1Sub_;
    mvvm::Observable<std::uint32_t>::Subscription color2Sub_;

    QSlider* diameterSlider_{nullptr};
    QLabel* diameterValueLabel_{nullptr};
    QPushButton* color1Button_{nullptr};
    QPushButton* color2Button_{nullptr};
};

#endif //CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
