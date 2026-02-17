// #include <iostream>
// #include <QApplication>
// #include <QTextStream>
// #include <QFile>
//
// #include "bootstrap/ApplicationBootstrap.h"
// #include "bootstrap/UiBootstrap.h"
// #include "bootstrap/UseCasesBootstrap.h"
// #include "bootstrap/ViewModelsBootstrap.h"
// #include "../domain/ports/angle/IAngleSource.h"
// #include "domain/ports/pressure/IPressureSource.h"
// #include "ui/widgets/QtMainWindow.h"
//
// namespace {
//     QString appStyle()
//     {
//         QFile file("../../../styles/style.qss");
//         if (!file.open(QFile::ReadOnly | QFile::Text)) {
//             return QString(); // или логирование ошибки
//         }
//
//         QTextStream in(&file);
//         return in.readAll();
//     }
// } // namespace
//
// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);
//
//     Q_INIT_RESOURCE(resources);
//
//     app.setStyleSheet(appStyle());
//
//     const std::string setup_dir = (argc > 1) ? argv[1] : "../../../setup";
//     const std::string catalog_dir = (argc > 1) ? argv[1] : "../../../catalogs";
//     const std::string log_dir = (argc > 1) ? argv[1] : "../../../logs";
//
//     try {
//         ApplicationBootstrap application_bootstrap(setup_dir, catalog_dir, log_dir);
//         application_bootstrap.initialize();
//
//         UseCasesBootstrap use_cases_bootstrap(application_bootstrap);
//         use_cases_bootstrap.initialize();
//
//         ViewModelsBootstrap view_models_bootstrap(application_bootstrap, use_cases_bootstrap);
//         view_models_bootstrap.initialize();
//
//         UiBootstrap ui_bootstrap(view_models_bootstrap);
//         ui_bootstrap.initialize();
//
//         ui_bootstrap.main_window->show();
//
//         application_bootstrap.angle_sources[0]->start();
//         // application_bootstrap.angle_sources[1]->start();
//
//         application_bootstrap.pressure_source->start();
//
//         return app.exec();
//     }
//     catch (const std::exception& e) {
//         std::cerr << e.what() << std::endl;
//     }
// }
