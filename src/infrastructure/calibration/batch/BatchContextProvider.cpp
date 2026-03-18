#include "BatchContextProvider.h"

#include <QDate>
#include <QDir>

#include "application/orchestrators/settings/CalibrationContextProvider.h"

infra::calib::BatchContextProvider::~BatchContextProvider() {
}

infra::calib::BatchContextProvider::BatchContextProvider(BatchContextProviderPorts ports)
    : logger_(ports.logger_)
    , context_provider_(ports.context_provider_)
{
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::previewNext() const
{
    return buildNextContext(false);
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::allocateNext()
{
    return buildNextContext(true);
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::buildNextContext(const bool create_directory) const
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

    QDir base_dir(QString::fromStdString(path));
    if (!base_dir.exists() && !base_dir.mkpath(".")) {
        logger_.error("Failed to create directory {}", path);
        return std::nullopt;
    }

    constexpr int max_attempts = 10000;
    for (int party_id = 1; party_id < max_attempts; ++party_id) {
        const std::string full_path = path + "/p" + std::to_string(party_id) + "-" + std::to_string(displacement_id);
        const QDir candidate_dir(QString::fromStdString(full_path));

        if (candidate_dir.exists()) {
            continue;
        }

        if (create_directory) {
            QDir parent_dir;
            if (!parent_dir.mkdir(QString::fromStdString(full_path))) {
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
