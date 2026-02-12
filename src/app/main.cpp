#include <iostream>
#include <QApplication>
#include <QTextStream>
#include <QFile>

#include "app/bootstrap/AppBootstrap.h"
#include "ui/widgets/QtMainWindow.h"

namespace {
    QString appStyle()
    {
        QFile file("../../../styles/style.qss");
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

    const std::string setup_dir = (argc > 1) ? argv[1] : "../../../setup";
    const std::string catalog_dir = (argc > 1) ? argv[1] : "../../../catalog";
    const std::string log_dir = (argc > 1) ? argv[1] : "../../../logs";

    try {
        AppBootstrap bootstrap(setup_dir, catalog_dir, log_dir);
        bootstrap.initialize();
        auto& window = bootstrap.mainWindow();
        window.show();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return app.exec();
}
