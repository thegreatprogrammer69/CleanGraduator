#include <QApplication>

#include "app/bootstrap/AppBootstrap.h"

namespace {
QString appStyle()
{
    return R"QSS(
QMainWindow, QWidget#centralWidget {
    background: #F3F4F6;
    color: #111827;
    font-family: "Segoe UI", "Inter", "Arial";
    font-size: 13px;
}
)QSS";
}
} // namespace

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(appStyle());

    const std::string configDirectory = (argc > 1) ? argv[1] : "config/bootstrap";

    AppBootstrap bootstrap(configDirectory);
    bootstrap.initialize();

    auto& window = bootstrap.mainWindow();
    window.show();

    return app.exec();
}
