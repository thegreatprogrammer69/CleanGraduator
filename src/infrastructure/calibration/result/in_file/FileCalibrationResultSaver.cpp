#include "FileCalibrationResultSaver.h"

#include <fstream>

namespace {
    void write_result(const domain::common::CalibrationResult& result, const std::string& path, const std::string& gauge_name) {
        for (const auto& source_id : result.sources()) {
            std::ofstream file(path + "/scale" + std::to_string(source_id.value) + ".tbl");
            if (!file.is_open()) break;
            file << "       320       240       230";
            file << gauge_name;
            for (const auto& point_id : result.points()) {
                domain::common::CalibrationCellKey key;
                key.source_id = source_id;
                key.point_id = point_id;
                file << point_id.pressure << " ";
                key.direction = domain::common::MotorDirection::Forward;
                if (auto val = result.cell(key))
                    if (val->angle()) file << *val->angle() << " ";
                key.direction = domain::common::MotorDirection::Backward;
                if (auto val = result.cell(key))
                    if (val->angle()) file << *val->angle() << " ";
                file << "0.0 0.0";
            }
        }
    }
}

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : ports_(ports) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(const domain::common::CalibrationResult& result) {
        if (!ports_.settings_query_.currentGaugeName()) return {};
        if (!ports_.settings_query_.currentSaveResultPath()) return {};
        std::string path = *ports_.settings_query_.currentSaveResultPath();
        std::string gauge_name = *ports_.settings_query_.currentGaugeName();
        write_result(result, path, gauge_name);
        return {true, ""};
    }

}