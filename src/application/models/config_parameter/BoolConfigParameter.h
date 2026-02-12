#ifndef CLEANGRADUATOR_BOOLCONFIGPARAMETER_H
#define CLEANGRADUATOR_BOOLCONFIGPARAMETER_H

#include "ConfigParameter.h"

namespace application::models {
    struct BoolConfigParameter : ConfigParameter {
        bool default_value;

        BoolConfigParameter(std::string n,
                            std::string d,
                            bool def)
            : ConfigParameter(std::move(n), std::move(d))
            , default_value(def) {}
    };
}

#endif //CLEANGRADUATOR_BOOLCONFIGPARAMETER_H