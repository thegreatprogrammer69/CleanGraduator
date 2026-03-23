#ifndef CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H

#include <QWidget>

#include <cstdint>

#include "viewmodels/Observable.h"

namespace mvvm {
class CircleOverlaySettingsViewModel;
}

class QPushButton;
class QSpinBox;

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
    void updateColorButton(QPushButton* button, std::uint32_t rgba);

private:
    mvvm::CircleOverlaySettingsViewModel& model_;
    mvvm::Observable<int>::Subscription diameterSub_;
    mvvm::Observable<std::uint32_t>::Subscription color1Sub_;
    mvvm::Observable<std::uint32_t>::Subscription color2Sub_;

    QSpinBox* diameterSpinBox_{nullptr};
    QPushButton* color1Button_{nullptr};
    QPushButton* color2Button_{nullptr};
};

#endif // CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
