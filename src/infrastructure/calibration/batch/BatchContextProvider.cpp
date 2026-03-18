#include "BatchContextProvider.h"

#include <QDate>
#include <QDir>

#include "application/orchestrators/settings/CalibrationContextProvider.h"

namespace {
    std::optional<std::string> resolve_base_path(
        application::orchestrators::CalibrationContextProvider& context_provider,
        fmt::Logger& logger)
    {
        const auto calibration_context = context_provider.current();
        if (!calibration_context) {
            logger.warn("BatchContextProvider: calibration context is not set");
            return std::nullopt;
        }

        const std::string base_path = calibration_context->batch_path();
        const std::string date = QDate::currentDate().toString("dd.MM.yyyy").toStdString();
        const std::string path = base_path + "/" + date;

        QDir base_dir(QString::fromStdString(path));
        if (!base_dir.exists() && !base_dir.mkpath(".")) {
            logger.error("Failed to create directory {}", path);
            return std::nullopt;
        }

        return path;
    }

    std::optional<application::models::BatchContext> build_next_batch_context(
        application::orchestrators::CalibrationContextProvider& context_provider,
        fmt::Logger& logger)
    {
        const auto calibration_context = context_provider.current();
        if (!calibration_context) {
            logger.warn("BatchContextProvider: calibration context is not set");
            return std::nullopt;
        }

        const auto dated_path = resolve_base_path(context_provider, logger);
        if (!dated_path) {
            return std::nullopt;
        }

        const int displacement_id = calibration_context->displacement.id;
        constexpr int max_attempts = 10000;

        for (int party_id = 1; party_id < max_attempts; ++party_id) {
            const std::string full_path =
                *dated_path + "/p" + std::to_string(party_id) + "-" + std::to_string(displacement_id);

            if (!QDir(QString::fromStdString(full_path)).exists()) {
                application::models::BatchContext ctx;
                ctx.party_id = party_id;
                ctx.full_path = full_path;
                return ctx;
            }
        }

        logger.error("Failed to allocate party_id after {} attempts", max_attempts);
        return std::nullopt;
    }
}

infra::calib::BatchContextProvider::~BatchContextProvider() {
}

infra::calib::BatchContextProvider::BatchContextProvider(BatchContextProviderPorts ports)
    : logger_(ports.logger_)
    , context_provider_(ports.context_provider_)
{
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::peekNext()
{
    return build_next_batch_context(context_provider_, logger_);
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::createNext()
{
    const auto batch_context = build_next_batch_context(context_provider_, logger_);
    if (!batch_context) {
        return std::nullopt;
    }

    QDir dir;
    if (!dir.mkpath(QString::fromStdString(batch_context->full_path.string()))) {
        logger_.error("Failed to create batch directory {}", batch_context->full_path.string());
        return std::nullopt;
    }

    return batch_context;
}
