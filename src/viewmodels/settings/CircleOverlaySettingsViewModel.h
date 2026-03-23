#ifndef CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H

#include <cstdint>
#include <vector>

#include "viewmodels/Observable.h"

namespace mvvm {
class ICircleOverlayViewModel;

struct CircleOverlaySettingsViewModelDeps {
    std::vector<ICircleOverlayViewModel*> targets;
};

class CircleOverlaySettingsViewModel final {
public:
    explicit CircleOverlaySettingsViewModel(CircleOverlaySettingsViewModelDeps deps);

    Observable<float> diameter_percent{0.0f};
    Observable<std::uint32_t> color1{0xFFFFFFFFu};
    Observable<std::uint32_t> color2{0x000000FFu};

    void setDiameterPercent(float value);
    void setColor1(std::uint32_t value);
    void setColor2(std::uint32_t value);

private:
    void applyToTargets() const;

private:
    CircleOverlaySettingsViewModelDeps deps_;
};

}

#endif //CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
