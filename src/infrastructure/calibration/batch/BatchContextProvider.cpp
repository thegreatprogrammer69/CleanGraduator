#include "BatchContextProvider.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"
#include <QDate>
#include <QDir>

#include "application/models/info/Displacement.h"

namespace {
    std::optional<application::models::BatchContext> resolve_batch_context(
        fmt::Logger& logger,
        application::orchestrators::CalibrationSettingsQuery& settings_query,
        bool create_directory)
    {
        if (!settings_query.currentBatchPath()) {
            logger.warn("BatchContextProvider: batch path is not set");
            return std::nullopt;
        }

        if (!settings_query.currentDisplacement()) {
            logger.warn("BatchContextProvider: displacement is not set");
            return std::nullopt;
        }

        const std::string base_path = *settings_query.currentBatchPath();
        const std::string date = QDate::currentDate().toString("dd.MM.yyyy").toStdString();
        const std::string path = base_path + "/" + date;

        const int displacement_id = *settings_query.currentDisplacement()->id;

        QDir base_dir(QString::fromStdString(path));
        if (!base_dir.exists() && !base_dir.mkpath(".")) {
            logger.error("Failed to create directory {}", path);
            return std::nullopt;
        }

        constexpr int max_attempts = 10000;
        for (int party_id = 1; party_id < max_attempts; ++party_id) {
            const std::string full_path = path + "/p" + std::to_string(party_id) + "-" + std::to_string(displacement_id);
            const QString qt_full_path = QString::fromStdString(full_path);

            if (QDir(qt_full_path).exists()) {
                continue;
            }

            if (create_directory) {
                QDir root;
                if (!root.mkpath(qt_full_path)) {
                    logger.error("Failed to create batch directory {}", full_path);
                    return std::nullopt;
                }
            }

            application::models::BatchContext ctx;
            ctx.party_id = party_id;
            ctx.full_path = full_path;
            return ctx;
        }

        logger.error("Failed to allocate party_id after {} attempts", max_attempts);
        return std::nullopt;
    }
}

infra::calib::BatchContextProvider::~BatchContextProvider() = default;

infra::calib::BatchContextProvider::BatchContextProvider(BatchContextProviderPorts ports)
    : logger_(ports.logger_), settings_query_(ports.settings_query_) {}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::previewNext() const {
    return resolve_batch_context(const_cast<fmt::Logger&>(logger_), settings_query_, false);
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::allocateNext() {
    return resolve_batch_context(logger_, settings_query_, true);
}
