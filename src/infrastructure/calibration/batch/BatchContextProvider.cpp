#include "BatchContextProvider.h"

#include <QDate>
#include <QDir>

#include "application/orchestrators/settings/CalibrationContextProvider.h"

infra::calib::BatchContextProvider::~BatchContextProvider() = default;

infra::calib::BatchContextProvider::BatchContextProvider(BatchContextProviderPorts ports)
    : logger_(ports.logger_)
    , context_provider_(ports.context_provider_)
{
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::previewNext() const
{
    return resolveNextBatch(false);
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::allocateNext()
{
    return resolveNextBatch(true);
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::resolveNextBatch(bool create_directory) const
{
    const auto calibration_context = context_provider_.current();
    if (!calibration_context) {
        logger_.warn("BatchContextProvider: calibration context is not set");
        return std::nullopt;
    }

    const std::string base_path = calibration_context->batch_path();
    const std::string date = QDate::currentDate().toString("dd.MM.yyyy").toStdString();
    const std::string path = base_path + "/" + date;
    const int displacement_id = calibration_context->displacement.id;

    QDir baseDir(QString::fromStdString(path));
    if (!baseDir.exists() && !baseDir.mkpath(".")) {
        logger_.error("Failed to create directory {}", path);
        return std::nullopt;
    }

    constexpr int max_attempts = 10000;
    for (int party_id = 1; party_id < max_attempts; ++party_id) {
        const std::string full_path = path + "/p" + std::to_string(party_id) + "-" + std::to_string(displacement_id);
        const QString full_path_q = QString::fromStdString(full_path);
        const bool exists = QDir(full_path_q).exists();

        if (exists) {
            continue;
        }

        if (create_directory) {
            QDir attempt;
            if (!attempt.mkdir(full_path_q)) {
                continue;
            }
        }

        application::models::BatchContext ctx;
        ctx.party_id = party_id;
        ctx.full_path = full_path;
        return ctx;
    }

    logger_.error("Failed to allocate party_id after {} attempts", max_attempts);
    return std::nullopt;
}
