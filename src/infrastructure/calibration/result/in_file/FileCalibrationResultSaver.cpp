#include "FileCalibrationResultSaver.h"

#include <fstream>
#include <optional>

namespace {

    constexpr double deg_to_rad(double deg) {
        return deg * 3.141592653589793 / 180.0;
    }

    void write_result(const domain::common::CalibrationResult& result,
                      const std::filesystem::path& path,
                      const std::optional<std::vector<domain::common::SourceId>>& selected_sources)
    {
        const auto& sources = selected_sources.has_value() ? *selected_sources : result.sources();
        for (const auto& source_id : sources) {

            std::ofstream file(path / ("scale" + std::to_string(source_id.value) + ".tbl"));
            if (!file.is_open())
                continue;

            file << "       320       240       230" << std::endl;
            file << result.gauge().name << std::endl;

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
                        else {
                            file << "0.0 ";
                        }
                    }
                    else {
                        file << "0.0 ";
                    }
                };

                write_angle(domain::common::MotorDirection::Forward);
                write_angle(domain::common::MotorDirection::Backward);

                file << "0.0 0.0" << std::endl;
            }
        }
    }

} // namespace

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : logger_(ports.logger) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(const domain::common::CalibrationResult& result,
                                     const std::filesystem::path& directory,
                                     const std::optional<std::vector<domain::common::SourceId>>& source_ids) {
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec) {
            logger_.error("Failed to create calibration result directory {}: {}", directory.string(), ec.message());
            return {false, "Не удалось создать директорию для сохранения.", {}};
        }

        write_result(result, directory, source_ids);
        return {true, "", directory};
    }

}
