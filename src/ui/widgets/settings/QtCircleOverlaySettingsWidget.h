#ifndef CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
#define CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H

#include <QWidget>

#include "viewmodels/Observable.h"

namespace mvvm {
class CircleOverlaySettingsViewModel;
}

class QSpinBox;
class QPushButton;

class QtCircleOverlaySettingsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit QtCircleOverlaySettingsWidget(
        mvvm::CircleOverlaySettingsViewModel& model,
        QWidget* parent = nullptr);

    ~QtCircleOverlaySettingsWidget() override;

private:
    void buildUi();
    void connectUi();
    void connectViewModel();
    void updateColorButton(QPushButton* button, std::uint32_t color) const;

private:
    mvvm::CircleOverlaySettingsViewModel& model_;
    mvvm::Observable<int>::Subscription diameter_sub_;
    mvvm::Observable<std::uint32_t>::Subscription color1_sub_;
    mvvm::Observable<std::uint32_t>::Subscription color2_sub_;

    QSpinBox* diameter_spin_{nullptr};
    QPushButton* color1_button_{nullptr};
    QPushButton* color2_button_{nullptr};
};

#endif //CLEANGRADUATOR_QTCIRCLEOVERLAYSETTINGSWIDGET_H
