#include "CircleOverlaySettingsViewModel.h"

#include <algorithm>

using namespace mvvm;

namespace {
std::uint32_t normalizeColor(std::uint32_t value)
{
    return value | 0x000000FFu;
}
}

CircleOverlaySettingsViewModel::CircleOverlaySettingsViewModel(CircleOverlaySettingsViewModelDeps deps)
    : targets_(std::move(deps.targets))
{
    if (!targets_.empty() && targets_.front()) {
        diameter_percent_.set(targets_.front()->circleDiameterPercent().get_copy());
        color1_.set(targets_.front()->circleColor1().get_copy());
        color2_.set(targets_.front()->circleColor2().get_copy());
    }

    setupGuards();
    setupPropagation();
    propagateToTargets();
}

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

void CircleOverlaySettingsViewModel::setupGuards()
{
    diameter_percent_.set_before_change([](const int&, int& proposed) {
        proposed = std::clamp(proposed, 0, 100);
        return true;
    });

    color1_.set_before_change([](const std::uint32_t&, std::uint32_t& proposed) {
        proposed = normalizeColor(proposed);
        return true;
    });

    color2_.set_before_change([](const std::uint32_t&, std::uint32_t& proposed) {
        proposed = normalizeColor(proposed);
        return true;
    });
}

void CircleOverlaySettingsViewModel::setupPropagation()
{
    diameter_percent_.set_after_change([this](const auto&) {
        propagateToTargets();
    });

    color1_.set_after_change([this](const auto&) {
        propagateToTargets();
    });

    color2_.set_after_change([this](const auto&) {
        propagateToTargets();
    });
}

void CircleOverlaySettingsViewModel::propagateToTargets()
{
    const int diameter = diameter_percent_.get_copy();
    const std::uint32_t first = color1_.get_copy();
    const std::uint32_t second = color2_.get_copy();

    for (auto* target : targets_) {
        if (!target) {
            continue;
        }

        target->circleDiameterPercent().set(diameter);
        target->circleColor1().set(first);
        target->circleColor2().set(second);
    }
}
