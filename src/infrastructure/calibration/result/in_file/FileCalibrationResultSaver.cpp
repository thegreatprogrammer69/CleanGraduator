#include "FileCalibrationResultSaver.h"

#include <fstream>

#include <QDir>

#include "application/ports/batch/IBatchContextProvider.h"

namespace {

    constexpr double deg_to_rad(double deg) {
        return deg * 3.141592653589793 / 180.0;
    }

    bool write_result(const domain::common::CalibrationResult& result,
                      const std::filesystem::path& path)
    {
        for (const auto& source_id : result.sources()) {

            std::ofstream file(path / ("scale" + std::to_string(source_id.value) + ".tbl"));
            if (!file.is_open())
                return false;

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

        return true;
    }

} // namespace

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : logger_(ports.logger), batch_context_provider_(ports.batch_context_provider) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(const domain::common::CalibrationResult& result) {
        const auto batch = batch_context_provider_.allocateNext();
        if (!batch) {
            return {false, "Не удалось определить каталог партии для сохранения.", {}};
        }

        return writeToPath(result, batch->full_path);
    }

    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::saveAs(const domain::common::CalibrationResult& result, const std::filesystem::path& output_path) {
        return writeToPath(result, output_path);
    }

    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::writeToPath(const domain::common::CalibrationResult& result, const std::filesystem::path& output_path)
    {
        QDir dir(QString::fromStdString(output_path.string()));
        if (!dir.exists() && !dir.mkpath(".")) {
            logger_.error("Failed to create directory {}", output_path.string());
            return {false, "Не удалось создать папку для сохранения.", {}};
        }

        if (!write_result(result, output_path)) {
            logger_.error("Failed to write calibration result to {}", output_path.string());
            return {false, "Не удалось записать файлы результата.", {}};
        }

        return {true, "", output_path};
    }

}
