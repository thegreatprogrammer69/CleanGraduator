#include <QApplication>
#include <QTextStream>
#include <QFile>

#include "app/bootstrap/AppBootstrap.h"
#include "ui/widgets/QtMainWindow.h"

namespace {
    QString appStyle()
    {
        QFile file("../../../config/style.qss");
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            return QString(); // или логирование ошибки
        }

        QTextStream in(&file);
        return in.readAll();
    }
} // namespace

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(appStyle());

    const std::string configDirectory = (argc > 1) ? argv[1] : "../../../config/bootstrap";

    // try {
        AppBootstrap bootstrap(configDirectory);
        bootstrap.initialize();
        auto& window = bootstrap.mainWindow();
        window.show();
    // }
    // catch (const std::exception& e) {
    //     std::cerr << e.what() << std::endl;
    // }

    return app.exec();
}
