#include "FileCalibrationResultSaver.h"

#include <fstream>

namespace {

    constexpr double deg_to_rad(double deg) {
        return deg * 3.141592653589793 / 180.0;
    }

    application::ports::ICalibrationResultSaver::Result write_result(
        const domain::common::CalibrationResult& result,
        const std::filesystem::path& path)
    {
        std::error_code ec;
        std::filesystem::create_directories(path, ec);
        if (ec) {
            return { false, "Не удалось создать каталог для сохранения результата." };
        }

        for (const auto& source_id : result.sources()) {
            std::ofstream file(path / ("scale" + std::to_string(source_id.value) + ".tbl"));
            if (!file.is_open()) {
                return { false, "Не удалось открыть файл результата для записи." };
            }

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

        return { true, "" };
    }

} // namespace

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : logger_(ports.logger) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(
        const domain::common::CalibrationResult& result,
        const std::filesystem::path& directory) {
        const auto save_result = write_result(result, directory);
        if (!save_result.success) {
            logger_.error("Failed to save calibration result to {}: {}", directory.string(), save_result.error);
        }
        return save_result;
    }

}
