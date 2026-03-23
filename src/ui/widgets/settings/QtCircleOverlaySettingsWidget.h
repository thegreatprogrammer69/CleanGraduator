#ifndef CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H

#include <QWidget>
#include <cstdint>

#include "viewmodels/Observable.h"

namespace mvvm {
class CircleOverlaySettingsViewModel;
}

class QDoubleSpinBox;
class QPushButton;

class QtCircleOverlaySettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtCircleOverlaySettingsWidget(
        mvvm::CircleOverlaySettingsViewModel& model,
        QWidget* parent = nullptr
    );

    ~QtCircleOverlaySettingsWidget() override;

private:
    void buildUi();
    void connectUi();
    void connectViewModel();

    void updateButtonColor(QPushButton* button, std::uint32_t rgba);
    void chooseColor1();
    void chooseColor2();

private:
    mvvm::CircleOverlaySettingsViewModel& model_;
    mvvm::Observable<float>::Subscription diameter_sub_;
    mvvm::Observable<std::uint32_t>::Subscription color1_sub_;
    mvvm::Observable<std::uint32_t>::Subscription color2_sub_;

    QDoubleSpinBox* diameterSpin_{nullptr};
    QPushButton* color1Button_{nullptr};
    QPushButton* color2Button_{nullptr};
};

#endif //CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
