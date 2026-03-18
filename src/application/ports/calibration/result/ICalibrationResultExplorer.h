#ifndef CLEANGRADUATOR_ICALIBRATIONRESULTEXPLORER_H
#define CLEANGRADUATOR_ICALIBRATIONRESULTEXPLORER_H
#include <filesystem>
#include <string>

namespace application::ports {

class ICalibrationResultExplorer {
public:
    struct Result {
        bool success;
        std::string error;
    };

    virtual ~ICalibrationResultExplorer() = default;
    virtual Result reveal(const std::filesystem::path& directory) = 0;
};

}

#endif //CLEANGRADUATOR_ICALIBRATIONRESULTEXPLORER_H
