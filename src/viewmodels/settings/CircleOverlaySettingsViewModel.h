#ifndef CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H

#include <cstdint>
#include <vector>

#include "viewmodels/video/ICircleOverlayConfigurable.h"

namespace mvvm {

struct CircleOverlaySettingsViewModelDeps {
    std::vector<ICircleOverlayConfigurable*> targets;
};

class CircleOverlaySettingsViewModel final : public ICircleOverlayConfigurable {
public:
    explicit CircleOverlaySettingsViewModel(CircleOverlaySettingsViewModelDeps deps);

    Observable<int>& circleDiameterPercent() override;
    const Observable<int>& circleDiameterPercent() const override;

    Observable<std::uint32_t>& circleColor1() override;
    const Observable<std::uint32_t>& circleColor1() const override;

    Observable<std::uint32_t>& circleColor2() override;
    const Observable<std::uint32_t>& circleColor2() const override;

private:
    void setupGuards();
    void setupPropagation();
    void propagateToTargets();

private:
    std::vector<ICircleOverlayConfigurable*> targets_;
    Observable<int> diameter_percent_{15};
    Observable<std::uint32_t> color1_{0xFFFFFFFFu};
    Observable<std::uint32_t> color2_{0x000000FFu};
};

}

#endif //CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
