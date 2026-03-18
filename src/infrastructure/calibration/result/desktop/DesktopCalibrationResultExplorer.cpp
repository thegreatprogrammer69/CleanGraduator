#include "DesktopCalibrationResultExplorer.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

namespace infra::calib {

application::ports::ICalibrationResultExplorer::Result
DesktopCalibrationResultExplorer::reveal(const std::filesystem::path& directory) {
    const QFileInfo info(QString::fromStdString(directory.string()));
    if (!info.exists()) {
        return {false, "Папка сохранения не найдена"};
    }

    const bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
    if (!ok) {
        return {false, "Не удалось открыть папку в проводнике"};
    }

    return {true, ""};
}

}
