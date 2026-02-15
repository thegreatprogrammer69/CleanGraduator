#include "ConfigureComponent.h"

#include "../../ports/outbound/settings/IRuntimeConfigurator.h"

using namespace application::usecase;
using namespace application::ports;
using namespace domain::ports;

ConfigureComponent::ConfigureComponent(ILogger &logger, IRuntimeConfigurator &configurator)
    : logger_(logger), configurator_(configurator)
{
    logger_.info("constructor called");
}

ConfigureComponent::~ConfigureComponent() {
    logger_.info("destructor called");
}

application::models::ConfigSchema ConfigureComponent::schema() const {
    return configurator_.schema();
}

void ConfigureComponent::setInt(const std::string &name, int v) {
    configurator_.setInt(name, v);
}

void ConfigureComponent::setBool(const std::string &name, bool v) {
    configurator_.setBool(name, v);
}

void ConfigureComponent::setText(const std::string &name, const std::string &v) {
    configurator_.setText(name, v);
}

void ConfigureComponent::invoke(const std::string &action) {
    configurator_.invoke(action);
}
