#ifndef CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
#define CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H

#include <cstdint>

#include "viewmodels/video/ICircleOverlaySettingsViewModel.h"

namespace mvvm {

class CircleOverlaySettingsViewModel final : public ICircleOverlaySettingsViewModel {
public:
    CircleOverlaySettingsViewModel() = default;
    ~CircleOverlaySettingsViewModel() override = default;

    Observable<int>& circleDiameterPercent() override;
    const Observable<int>& circleDiameterPercent() const override;

    Observable<std::uint32_t>& circleColor1() override;
    const Observable<std::uint32_t>& circleColor1() const override;

    Observable<std::uint32_t>& circleColor2() override;
    const Observable<std::uint32_t>& circleColor2() const override;

private:
    Observable<int> diameter_percent_{30};
    Observable<std::uint32_t> color1_{0xFFFFFFFFu};
    Observable<std::uint32_t> color2_{0x000000FFu};
};

}

#endif //CLEANGRADUATOR_CIRCLEOVERLAYSETTINGSVIEWMODEL_H
