#ifndef CLEANGRADUATOR_IRUNTIMECONFIGURATOR_H
#define CLEANGRADUATOR_IRUNTIMECONFIGURATOR_H
#include <any>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "application/models/config_parameter/ConfigParameter.h"

namespace application::ports {
    class IRuntimeConfigurator {
    public:
        virtual ~IRuntimeConfigurator() = default;

        virtual models::ConfigSchema schema() const = 0;

        virtual void setInt(const std::string& name, int value) = 0;
        virtual void setBool(const std::string& name, bool value) = 0;
        virtual void setText(const std::string& name, const std::string& value) = 0;

        virtual void invoke(const std::string& action) = 0;
    };
}

#endif //CLEANGRADUATOR_IRUNTIMECONFIGURATOR_H