#ifndef CLEANGRADUATOR_IVIDEOSOURCEFACTORY_H
#define CLEANGRADUATOR_IVIDEOSOURCEFACTORY_H
#include <memory>
#include <vector>

namespace domain::ports {
    struct IVideoSource;
}

namespace application::ports {
    struct IVideoSourcesFactory {
        virtual std::vector<std::unique_ptr<domain::ports::IVideoSource>> create() = 0;
        virtual ~IVideoSourcesFactory() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCEFACTORY_H