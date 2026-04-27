#include "BatchContextProvider.h"

#include <QDate>
#include <QDir>
#include <QFileInfo>
#include <QString>

#include "application/orchestrators/settings/CalibrationContextProvider.h"

infra::calib::BatchContextProvider::~BatchContextProvider() {}

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

    QString basePath = QString::fromStdString(calibration_context->batch_path());
    QString date = QDate::currentDate().toString("dd.MM.yyyy");

    QDir baseDir(basePath);

    logger_.info("Base path: {}", basePath.toStdString());
    logger_.info("Absolute base path: {}", baseDir.absolutePath().toStdString());

    if (!baseDir.exists()) {
        logger_.error("Base path does not exist: {}", basePath.toStdString());
        return std::nullopt;
    }

    QString fullBasePath = baseDir.filePath("stend4\\" + date);

    // --- диагностика перед созданием ---
    QDir checkDir(fullBasePath);
    if (!checkDir.exists()) {
        QDir parentDir = checkDir;
        QStringList missingParts;

        // ищем существующего родителя
        while (!parentDir.exists() && !parentDir.isRoot()) {
            missingParts.prepend(parentDir.dirName());
            if (!parentDir.cdUp())
                break;
        }

        if (!parentDir.exists()) {
            logger_.error("No existing parent directory found for '{}'",
                          fullBasePath.toStdString());
        } else {
            QFileInfo info(parentDir.absolutePath());

            QStringList problems;
            if (!info.isReadable())   problems << "not readable";
            if (!info.isWritable())   problems << "not writable";
            if (!info.isExecutable()) problems << "not accessible";

            if (!problems.isEmpty()) {
                logger_.error("Parent '{}' has permission issues: {}",
                              parentDir.absolutePath().toStdString(),
                              problems.join(", ").toStdString());
            }

            logger_.info("Nearest existing parent: {}",
                         parentDir.absolutePath().toStdString());

            if (!missingParts.isEmpty()) {
                logger_.info("Missing path parts: {}",
                             missingParts.join("\\").toStdString());
            }
        }
    }

    // --- создание пути ---
    QDir dir;
    if (!dir.mkpath(fullBasePath)) {
        logger_.error("Failed to create directory '{}'",
                      fullBasePath.toStdString());
        return std::nullopt;
    }

    int displacement_id = calibration_context->displacement.id;

    int party_id = 1;
    const int max_attempts = 64;

    while (party_id < max_attempts) {
        QString partyDirName =
            QString("p%1-%2").arg(party_id).arg(displacement_id);

        QString fullPath = QDir(fullBasePath).filePath(partyDirName);

        QDir attempt;
        if (attempt.mkdir(fullPath)) {
            application::models::BatchContext ctx;
            ctx.party_id = party_id;
            ctx.full_path = fullPath.toStdWString();

            logger_.info("Allocated party_id={}, path={}",
                         party_id, ctx.full_path);

            return ctx;
        }

        // если не создалось — логируем причину
        QFileInfo fi(fullPath);
        if (fi.exists()) {
            logger_.warn("Directory already exists: {}", fullPath.toStdString());
        } else {
            logger_.warn("Failed to create directory: {}", fullPath.toStdString());
        }

        party_id++;
    }

    logger_.error("Failed to allocate party_id after {} attempts", max_attempts);
    return std::nullopt;
}