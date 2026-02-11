#ifndef CLEANGRADUATOR_IVIDEOSOURCEREPOSITORY_H
#define CLEANGRADUATOR_IVIDEOSOURCEREPOSITORY_H
#include <memory>
#include <vector>

namespace domain::ports {
    struct IVideoSource;
}

namespace application::ports {
    class IVideoSourceRepository {
    public:
        virtual std::vector<domain::ports::IVideoSource*> all() = 0;
        virtual ~IVideoSourceRepository() = default;
    };
}

#endif //CLEANGRADUATOR_IVIDEOSOURCEREPOSITORY_H