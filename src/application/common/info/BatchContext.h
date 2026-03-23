#ifndef CLEANGRADUATOR_BATCHNUMBER_H
#define CLEANGRADUATOR_BATCHNUMBER_H
#include <filesystem>

namespace application::models {
    struct BatchContext {
        int party_id;
        std::filesystem::path full_path;
    };

}

#endif //CLEANGRADUATOR_BATCHNUMBER_H