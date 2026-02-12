#ifndef CLEANGRADUATOR_TEXTCONFIGPARAMETER_H
#define CLEANGRADUATOR_TEXTCONFIGPARAMETER_H
#include "ConfigParameter.h"

namespace application::models {
    struct TextConfigParameter : ConfigParameter {
        std::string default_value;

        TextConfigParameter(std::string n,
                            std::string d,
                            std::string def)
            : ConfigParameter(std::move(n), std::move(d))
            , default_value(std::move(def)) {}
    };

}

#endif //CLEANGRADUATOR_TEXTCONFIGPARAMETER_H