#ifndef CLEANGRADUATOR_BATCHNUMBER_H
#define CLEANGRADUATOR_BATCHNUMBER_H
#include <filesystem>

namespace application::models {
    struct BatchContext {
        int number;
        std::filesystem::path directory;
    };

}

#endif //CLEANGRADUATOR_BATCHNUMBER_H