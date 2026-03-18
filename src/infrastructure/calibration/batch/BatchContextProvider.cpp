#include "BatchContextProvider.h"
#include "application/orchestrators/settings/CalibrationSettingsQuery.h"
#include <QDate>
#include <QDir>

#include "application/models/info/Displacement.h"

infra::calib::BatchContextProvider::~BatchContextProvider() {
}

infra::calib::BatchContextProvider::BatchContextProvider(BatchContextProviderPorts ports)
    : logger_(ports.logger_), settings_query_(ports.settings_query_)
{
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::current() {
    return current_context_;
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::allocate() {
    current_context_ = createContext();
    return current_context_;
}

std::optional<application::models::BatchContext>
infra::calib::BatchContextProvider::createContext() {

    if (!settings_query_.currentBatchPath()) {
        logger_.warn("BatchContextProvider: batch path is not set");
        return std::nullopt;
    }

    if (!settings_query_.currentDisplacement()) {
        logger_.warn("BatchContextProvider: displacement is not set");
        return std::nullopt;
    }

    std::string base_path = *settings_query_.currentBatchPath();
    std::string date = QDate::currentDate().toString("dd.MM.yyyy").toStdString();
    std::string path = base_path + "/" + date;

    int displacement_id = *settings_query_.currentDisplacement()->id;

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
