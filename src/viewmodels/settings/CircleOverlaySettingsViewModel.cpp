#include "CircleOverlaySettingsViewModel.h"

#include <algorithm>

#include "viewmodels/video/ICircleOverlayViewModel.h"

using namespace mvvm;

namespace {
constexpr float kMinDiameterPercent = 0.0f;
constexpr float kMaxDiameterPercent = 100.0f;

float clampDiameterPercent(float value)
{
    return std::clamp(value, kMinDiameterPercent, kMaxDiameterPercent);
}
}

CircleOverlaySettingsViewModel::CircleOverlaySettingsViewModel(CircleOverlaySettingsViewModelDeps deps)
    : deps_(std::move(deps))
{
    if (!deps_.targets.empty() && deps_.targets.front() != nullptr) {
        auto* target = deps_.targets.front();
        diameter_percent.set(clampDiameterPercent(target->circleDiameterPercent().get_copy()));
        color1.set(target->circleColor1().get_copy());
        color2.set(target->circleColor2().get_copy());
    }

    applyToTargets();
}

void CircleOverlaySettingsViewModel::setDiameterPercent(float value)
{
    diameter_percent.set(clampDiameterPercent(value));
    applyToTargets();
}

void CircleOverlaySettingsViewModel::setColor1(std::uint32_t value)
{
    color1.set(value);
    applyToTargets();
}

void CircleOverlaySettingsViewModel::setColor2(std::uint32_t value)
{
    color2.set(value);
    applyToTargets();
}

void CircleOverlaySettingsViewModel::applyToTargets() const
{
    const float diameter = clampDiameterPercent(diameter_percent.get_copy());
    const std::uint32_t first = color1.get_copy();
    const std::uint32_t second = color2.get_copy();

    for (auto* target : deps_.targets) {
        if (target == nullptr) {
            continue;
        }

        target->circleDiameterPercent().set(diameter);
        target->circleColor1().set(first);
        target->circleColor2().set(second);
    }
}
