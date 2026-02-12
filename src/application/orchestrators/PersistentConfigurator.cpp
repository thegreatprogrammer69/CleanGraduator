#include "PersistentConfigurator.h"

#include <variant>

#include "application/ports/outbound/IComponentSettingsRepository.h"
#include "application/ports/outbound/IRuntimeConfigurator.h"

application::ineractors::PersistentConfigurator::PersistentConfigurator(
    ports::IRuntimeConfigurator &configurator,
    ports::IComponentSettingsRepository &repo)
        : configurator_(configurator)
        , repo_(repo)
{}

void application::ineractors::PersistentConfigurator::loadAndApply() {
    auto settings = repo_.load();

    for (auto& entry : settings.values)
    {
        const auto& name = entry.first;
        const auto& value = entry.second;

        std::visit([&](auto&& v) {
            using T = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<T, int>)
                configurator_.setInt(name, v);
            else if constexpr (std::is_same_v<T, bool>)
                configurator_.setBool(name, v);
            else if constexpr (std::is_same_v<T, std::string>)
                configurator_.setText(name, v);
        }, value);
    }
}



void application::ineractors::PersistentConfigurator::save(const models::ComponentSettings &settings) {
    repo_.save(settings);
}
