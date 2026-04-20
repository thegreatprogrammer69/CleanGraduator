#include <iostream>
#include <stdexcept>

#include <QApplication>
#include <QTextStream>
#include <QFile>
#include <QCoreApplication>
#include <QString>

#include "bootstrap/ApplicationBootstrap.h"
#include "bootstrap/UiBootstrap.h"
#include "bootstrap/UseCasesBootstrap.h"
#include "bootstrap/ViewModelsBootstrap.h"
#include "domain/ports/angle/IAngleSource.h"
#include "../domain/ports/drivers/motor/IMotorDriver.h"
#include "domain/ports/pressure/IPressureSource.h"
#include "ui/widgets/QtMainWindow.h"

namespace {
    QString loadStyleSheet(const QString& path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error(
                std::string("Cannot open style file: ") + path.toStdString()
            );
        }

        QTextStream in(&file);
        return in.readAll();
    }
}

#include <windows.h>
#include <DbgHelp.h>

void CreateDump(EXCEPTION_POINTERS* pep) {
    HANDLE hFile = CreateFile(L"crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = pep;
    mei.ClientPointers = FALSE;

    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        &mei,
        NULL,
        NULL
    );

    CloseHandle(hFile);
}

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    CreateDump(ExceptionInfo);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{
    SetUnhandledExceptionFilter(ExceptionHandler);

    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(resources);

    if (argc < 5) {
        std::cerr
            << "Usage: " << argv[0]
            << " <setup_dir> <catalog_dir> <log_dir> <style_qss_path>\n"
            << "Example:\n"
            << "  " << argv[0]
            << " ./setup ./catalogs ./logs ./styles/style.qss\n";
        return 1;
    }

    const std::string setup_dir   = argv[1];
    const std::string catalog_dir = argv[2];
    const std::string log_dir     = argv[3];
    const QString style_path      = QString::fromLocal8Bit(argv[4]);

    try {
        app.setStyle("Windows");
        app.setStyleSheet(loadStyleSheet(style_path));

        ApplicationBootstrap application_bootstrap(setup_dir, catalog_dir, log_dir);
        application_bootstrap.initialize();

        UseCasesBootstrap use_cases_bootstrap(application_bootstrap);
        use_cases_bootstrap.initialize();

        ViewModelsBootstrap view_models_bootstrap(application_bootstrap, use_cases_bootstrap);
        view_models_bootstrap.initialize();

        UiBootstrap ui_bootstrap(view_models_bootstrap);
        ui_bootstrap.initialize();

        ui_bootstrap.main_window->setMinimumSize(1280, 960);
        ui_bootstrap.main_window->show();

        return app.exec();
    }
    catch (const std::exception& e) {
        std::cerr << "Startup error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Startup error: unknown exception" << std::endl;
        return 1;
    }
}
