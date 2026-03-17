#include "FileCalibrationResultSaver.h"

#include <fstream>

#include "application/ports/batch/IBatchContextProvider.h"

namespace {

    constexpr double deg_to_rad(double deg) {
        return deg * 3.141592653589793 / 180.0;
    }

    void write_result(const domain::common::CalibrationResult& result,
                      const std::filesystem::path& path)
    {
        for (const auto& source_id : result.sources()) {

            std::ofstream file(path / ("scale" + std::to_string(source_id.value) + ".tbl"));
            if (!file.is_open())
                continue;

            file << "       320       240       230";
            file << result.gauge().name;

            for (const auto& point_id : result.points()) {

                domain::common::CalibrationCellKey key;
                key.source_id = source_id;
                key.point_id = point_id;

                file << point_id.pressure << " ";

                auto write_angle = [&](domain::common::MotorDirection dir) {
                    key.direction = dir;
                    if (auto val = result.cell(key)) {
                        if (val->angle()) {
                            file << deg_to_rad(*val->angle()) << " ";
                        }
                    }
                };

                write_angle(domain::common::MotorDirection::Forward);
                write_angle(domain::common::MotorDirection::Backward);

                file << "0.0 0.0";
            }
        }
    }

} // namespace

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : logger_(ports.logger), batch_context_provider_(ports.batch_context_provider) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(const domain::common::CalibrationResult& result) {
        write_result(result, batch_context_provider_.current()->full_path);
        return {true, ""};
    }

}