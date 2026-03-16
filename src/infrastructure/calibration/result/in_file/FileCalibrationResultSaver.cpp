#include "FileCalibrationResultSaver.h"

#include <fstream>

namespace {
    void write_result(const domain::common::CalibrationResult& result) {

    }
}

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : ports_(ports) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(const domain::common::CalibrationResult& result)
    {
        auto pathOpt = ports_.settings_query_.currentSaveResultPath();

        if (!pathOpt) {
            return {false, "Save path is not set"};
        }

        const auto& path = *pathOpt;

        std::ofstream file(path);

        if (!file.is_open()) {
            return {false, "Failed to open file: " + path.string()};
        }

        file << result;

        if (!file.good()) {
            return {false, "Failed to write calibration result"};
        }

        return {true, ""};
    }

}