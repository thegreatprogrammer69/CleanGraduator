#ifndef CLEANGRADUATOR_CONFIGURECOMPONENT_H
#define CLEANGRADUATOR_CONFIGURECOMPONENT_H
#include "domain/fmt/Logger.h"
#include "application/ports/settings/IRuntimeConfigurator.h"

namespace domain::ports {
    struct ILogger;
}


namespace application::usecase {
    class ConfigureComponent {
    public:
        ConfigureComponent(domain::ports::ILogger& logger, ports::IRuntimeConfigurator& configurator);
        ~ConfigureComponent();

        models::ConfigSchema schema() const;

        void setInt(const std::string& name, int v);
        void setBool(const std::string& name, bool v);
        void setText(const std::string& name, const std::string& v);
        void invoke(const std::string& action);

    private:
        fmt::Logger logger_;
        ports::IRuntimeConfigurator& configurator_;
    };
}

#endif //CLEANGRADUATOR_CONFIGURECOMPONENT_H