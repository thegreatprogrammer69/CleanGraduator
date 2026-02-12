#ifndef CLEANGRADUATOR_COMPONENTSETTINGS_H
#define CLEANGRADUATOR_COMPONENTSETTINGS_H

#include <string>
#include <unordered_map>
#include <variant>

namespace application::models {

    using SettingValue = std::variant<int, bool, std::string>;

    struct ComponentSettings {
        std::unordered_map<std::string, SettingValue> values;
    };

} // namespace application::settings

#endif //CLEANGRADUATOR_COMPONENTSETTINGS_H