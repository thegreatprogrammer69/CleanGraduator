#ifndef CLEANGRADUATOR_ACTIONCONFIGPARAMETER_H
#define CLEANGRADUATOR_ACTIONCONFIGPARAMETER_H
#include "ConfigParameter.h"

namespace application::models {
    struct ActionConfigParameter : application::models::ConfigParameter {
        ActionConfigParameter(std::string n,
                              std::string d)
            : ConfigParameter(std::move(n), std::move(d)) {}
    };
}

#endif //CLEANGRADUATOR_ACTIONCONFIGPARAMETER_H