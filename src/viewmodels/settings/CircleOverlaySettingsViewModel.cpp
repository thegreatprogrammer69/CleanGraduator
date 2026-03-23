#include "CircleOverlaySettingsViewModel.h"

using namespace mvvm;

Observable<int>& CircleOverlaySettingsViewModel::circleDiameterPercent()
{
    return diameter_percent_;
}

const Observable<int>& CircleOverlaySettingsViewModel::circleDiameterPercent() const
{
    return diameter_percent_;
}

Observable<std::uint32_t>& CircleOverlaySettingsViewModel::circleColor1()
{
    return color1_;
}

const Observable<std::uint32_t>& CircleOverlaySettingsViewModel::circleColor1() const
{
    return color1_;
}

Observable<std::uint32_t>& CircleOverlaySettingsViewModel::circleColor2()
{
    return color2_;
}

const Observable<std::uint32_t>& CircleOverlaySettingsViewModel::circleColor2() const
{
    return color2_;
}
