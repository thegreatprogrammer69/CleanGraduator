#include "BatchContextProvider.h"

#include <QDate>
#include <QDir>
#include <QRegularExpression>

#include "application/orchestrators/settings/CalibrationContextProvider.h"

infra::calib::BatchContextProvider::~BatchContextProvider() {
}

infra::calib::BatchContextProvider::BatchContextProvider(BatchContextProviderPorts ports)
    : logger_(ports.logger_)
    , context_provider_(ports.context_provider_)
{
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::current()
{
    const auto calibration_context = context_provider_.current();
    if (!calibration_context) {
        logger_.warn("BatchContextProvider: calibration context is not set");
        return std::nullopt;
    }

    std::string base_path = calibration_context->batch_path();
    std::string date = QDate::currentDate().toString("dd.MM.yyyy").toStdString();
    std::string path = base_path + "\\" + "stend4" + "\\" + date;

    int displacement_id = calibration_context->displacement.id;

    QDir baseDir(QString::fromStdString(path));
    if (!baseDir.exists() && !baseDir.mkpath(".")) {
        logger_.error("Failed to create directory {}", path);
        return std::nullopt;
    }

    int party_id = 1;
    std::string full_path;

    const int max_attempts = 10000;

    while (party_id < max_attempts) {
        full_path = path + "/p" +
                    std::to_string(party_id) +
                    "-" +
                    std::to_string(displacement_id);

        QDir attempt;
        if (attempt.mkdir(QString::fromStdString(full_path))) {
            application::models::BatchContext ctx;
            ctx.party_id = party_id;
            ctx.full_path = full_path;
            return ctx;
        }

        party_id++;
    }

    logger_.error("Failed to allocate party_id after {} attempts", max_attempts);
    return std::nullopt;
}
