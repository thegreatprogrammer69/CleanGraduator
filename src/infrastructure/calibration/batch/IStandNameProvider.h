#ifndef CLEANGRADUATOR_ISTANDNAMEPROVIDER_H
#define CLEANGRADUATOR_ISTANDNAMEPROVIDER_H

#include <optional>
#include <string>

namespace infra::calib {
    struct IStandNameProvider {
        virtual ~IStandNameProvider() = default;
        virtual std::optional<std::string> current() = 0;
    };
}

#endif //CLEANGRADUATOR_ISTANDNAMEPROVIDER_H
