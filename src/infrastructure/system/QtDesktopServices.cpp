#include "QtDesktopServices.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QUrl>

application::ports::IDesktopServices::Result
infra::system::QtDesktopServices::revealInFileManager(const std::filesystem::path& path) {
    const QFileInfo info(QString::fromStdString(path.string()));
    const QString absolute_path = info.exists() ? info.absoluteFilePath() : QDir(QString::fromStdString(path.string())).absolutePath();

    if (absolute_path.isEmpty()) {
        return {false, "Не удалось определить путь для открытия"};
    }

    const bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(absolute_path));
    if (!opened) {
        return {false, "Не удалось открыть папку в проводнике"};
    }

    return {true, {}};
}
