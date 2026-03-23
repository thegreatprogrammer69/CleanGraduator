#include "CircleOverlaySettingsViewModel.h"

#include <algorithm>
#include <utility>

#include "viewmodels/video/CircleOverlaySettings.h"
#include "viewmodels/video/ICircleOverlayConfigurable.h"

using namespace mvvm;

namespace {
int clampPercent(int value)
{
    return std::clamp(value, 0, 100);
}
}

CircleOverlaySettingsViewModel::CircleOverlaySettingsViewModel(CircleOverlaySettingsViewModelDeps deps)
    : deps_(std::move(deps))
{
    CircleOverlaySettings initial{};
    if (!deps_.configurables.empty() && deps_.configurables.front() != nullptr) {
        initial = deps_.configurables.front()->circleOverlaySettings().get_copy();
    }

    diameterPercent.set(clampPercent(initial.diameter_percent));
    color1.set(initial.color1);
    color2.set(initial.color2);
}

CircleOverlaySettingsViewModel::~CircleOverlaySettingsViewModel() = default;

void CircleOverlaySettingsViewModel::setDiameterPercent(int value)
{
    diameterPercent.set(clampPercent(value));
    applyToConfigurables();
}

void CircleOverlaySettingsViewModel::setColor1(std::uint32_t value)
{
    color1.set(value);
    applyToConfigurables();
}

void CircleOverlaySettingsViewModel::setColor2(std::uint32_t value)
{
    color2.set(value);
    applyToConfigurables();
}

void CircleOverlaySettingsViewModel::applyToConfigurables()
{
    const CircleOverlaySettings settings{
        clampPercent(diameterPercent.get_copy()),
        color1.get_copy(),
        color2.get_copy()
    };

    for (auto* configurable : deps_.configurables) {
        if (!configurable) {
            continue;
        }

        configurable->circleOverlaySettings().set(settings);
    }
}
