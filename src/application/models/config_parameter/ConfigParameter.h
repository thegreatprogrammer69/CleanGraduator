#ifndef CLEANGRADUATOR_CONFIGPARAMETER_H
#define CLEANGRADUATOR_CONFIGPARAMETER_H
#include <memory>
#include <string>
#include <vector>

#include "LayoutHint.h"

namespace application::models {
    struct ConfigParameter {
        std::string name;
        std::string description;
        LayoutHint layout;

        explicit ConfigParameter(std::string n,
                                 std::string d,
                                 LayoutHint l = {})
            : name(std::move(n))
            , description(std::move(d))
            , layout(l)
        {}

        virtual ~ConfigParameter() = default;
    };

    using ConfigSchema = std::vector<ConfigParameter*>;

}

inline bool operator==(const application::models::ConfigSchema& lhs, const application::models::ConfigSchema& rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (int i = 0; i < lhs.size(); ++i)
    {
        if (lhs[i].get() != rhs[i].get())
            return false;
    }

    return true;
}

#endif //CLEANGRADUATOR_CONFIGPARAMETER_H