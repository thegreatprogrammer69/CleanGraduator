#include "FileCalibrationResultSaver.h"

#include <fstream>

#include "application/ports/batch/IBatchContextProvider.h"

namespace {

    constexpr double deg_to_rad(double deg) {
        return deg * 3.141592653589793 / 180.0;
    }

    bool write_result(const domain::common::CalibrationResult& result,
                      const std::filesystem::path& path)
    {
        std::error_code error_code;
        std::filesystem::create_directories(path, error_code);
        if (error_code) {
            return false;
        }

        bool wrote_any_file = false;
        for (const auto& source_id : result.sources()) {
            std::ofstream file(path / ("scale" + std::to_string(source_id.value) + ".tbl"));
            if (!file.is_open()) {
                return false;
            }

            wrote_any_file = true;
            file << "       320       240       230";
            file << result.gauge().name;

            for (const auto& point_id : result.points()) {
                domain::common::CalibrationCellKey key;
                key.source_id = source_id;
                key.point_id = point_id;

                file << point_id.pressure << " ";

                const auto write_angle = [&](domain::common::MotorDirection dir) {
                    key.direction = dir;
                    if (const auto val = result.cell(key); val && val->angle()) {
                        file << deg_to_rad(*val->angle()) << " ";
                    }
                };

                write_angle(domain::common::MotorDirection::Forward);
                write_angle(domain::common::MotorDirection::Backward);

                file << "0.0 0.0";
            }
        }

        return wrote_any_file;
    }

} // namespace

namespace infra::calib {

    FileCalibrationResultSaver::FileCalibrationResultSaver(CalibrationResultSaverPorts ports)
        : logger_(ports.logger), batch_context_provider_(ports.batch_context_provider) {}


    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::save(const domain::common::CalibrationResult& result) {
        const auto batch_context = batch_context_provider_.allocateNext();
        if (!batch_context) {
            return {false, "Не удалось подготовить директорию для сохранения", std::nullopt, std::nullopt};
        }

        auto save_result = saveToDirectory(result, batch_context->full_path);
        save_result.party_id = batch_context->party_id;
        save_result.directory = batch_context->full_path;
        return save_result;
    }

    application::ports::ICalibrationResultSaver::Result
    FileCalibrationResultSaver::saveToDirectory(
        const domain::common::CalibrationResult& result,
        const std::filesystem::path& directory)
    {
        if (!write_result(result, directory)) {
            logger_.error("Failed to save calibration result to {}", directory.string());
            return {false, "Не удалось сохранить файлы результата", std::nullopt, directory};
        }

        return {true, "", std::nullopt, directory};
    }

}