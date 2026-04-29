#include "FileStandNameProvider.h"

#include <QFile>
#include <QTextStream>

namespace {
    QString normalizeStandName(QString value) {
        return value.trimmed();
    }
}

infra::calib::FileStandNameProvider::FileStandNameProvider(domain::ports::ILogger& logger, QString stand_file_path)
    : logger_(logger)
    , stand_file_path_(std::move(stand_file_path))
{
}

std::optional<std::string> infra::calib::FileStandNameProvider::current()
{
    QFile file(stand_file_path_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logger_.error("Failed to open stand file '{}': {}",
                      stand_file_path_.toStdString(),
                      file.errorString().toStdString());
        return std::nullopt;
    }

    QTextStream stream(&file);
    const auto standName = normalizeStandName(stream.readLine());

    if (standName.isEmpty()) {
        logger_.error("Stand file '{}' is empty", stand_file_path_.toStdString());
        return std::nullopt;
    }

    return standName.toStdString();
}
