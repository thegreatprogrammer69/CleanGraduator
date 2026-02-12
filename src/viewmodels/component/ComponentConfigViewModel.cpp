#include "ComponentConfigViewModel.h"

using namespace mvvm;
using namespace application::usecase;
using namespace application::models;

ComponentConfigViewModel::ComponentConfigViewModel(ConfigureComponent& usecase)
    : usecase_(usecase)
{}

void ComponentConfigViewModel::load()
{
    schema.set(usecase_.schema());
}

void ComponentConfigViewModel::setInt(const std::string& name, int value)
{
    usecase_.setInt(name, value);
}

void ComponentConfigViewModel::setBool(const std::string& name, bool value)
{
    usecase_.setBool(name, value);
}

void ComponentConfigViewModel::setText(const std::string& name, const std::string& value)
{
    usecase_.setText(name, value);
}

void ComponentConfigViewModel::invoke(const std::string& action)
{
    usecase_.invoke(action);
}