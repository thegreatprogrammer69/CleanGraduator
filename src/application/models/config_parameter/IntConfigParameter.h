#ifndef CLEANGRADUATOR_INTCONFIGPARAMETER_H
#define CLEANGRADUATOR_INTCONFIGPARAMETER_H
#include "ConfigParameter.h"

namespace application::models {
    struct IntConfigParameter : ConfigParameter {
        int default_value;
        int min;
        int max;

        IntConfigParameter(std::string n,
                           std::string d,
                           int def,
                           int minv,
                           int maxv)
            : ConfigParameter(std::move(n), std::move(d))
            , default_value(def)
            , min(minv)
            , max(maxv) {}
    };
}


#endif //CLEANGRADUATOR_INTCONFIGPARAMETER_H