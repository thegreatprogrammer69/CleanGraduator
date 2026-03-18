#include "QtFileExplorerLauncher.h"

#include <QDesktopServices>
#include <QDir>
#include <QUrl>

namespace infra::desktop {

application::ports::IFileExplorerLauncher::Result QtFileExplorerLauncher::showInExplorer(
    const std::filesystem::path& path)
{
    const auto absolute_path = std::filesystem::absolute(path);
    const auto url = QUrl::fromLocalFile(QString::fromStdString(absolute_path.string()));

    if (!QDesktopServices::openUrl(url)) {
        return {false, "Не удалось открыть проводник для сохранённой партии."};
    }

    return {true, {}};
}

}
