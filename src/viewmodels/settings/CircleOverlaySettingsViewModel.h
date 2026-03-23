#ifndef CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H

#include <cstdint>
#include <vector>

#include "viewmodels/Observable.h"

namespace mvvm {

class ICircleOverlayConfigurable;

struct CircleOverlaySettingsViewModelDeps {
    std::vector<ICircleOverlayConfigurable*> configurables;
};

class CircleOverlaySettingsViewModel {
public:
    explicit CircleOverlaySettingsViewModel(CircleOverlaySettingsViewModelDeps deps);
    ~CircleOverlaySettingsViewModel();

    Observable<int> diameterPercent;
    Observable<std::uint32_t> color1;
    Observable<std::uint32_t> color2;

    void setDiameterPercent(int value);
    void setColor1(std::uint32_t value);
    void setColor2(std::uint32_t value);

private:
    void applyToConfigurables();

private:
    CircleOverlaySettingsViewModelDeps deps_;
};

}

#endif // CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
