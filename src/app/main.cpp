#include <infrastructure/camera/windows/dshow/DShowCamera.h>
#include "application/fmt/fmt.h"
#include <infrastructure/camera/linux/v4l/V4LCamera.h>
#include "infrastructure/camera/linux/gstreamer/GStreamerCamera.h"

#include <infrastructure/logging/ConsoleLogger.h>

#include <infrastructure/clock/SessionClock.h>

#include <infrastructure/catalogs/FilePrinterCatalog.h>
#include <infrastructure/catalogs/FileDisplacementCatalog.h>
#include <infrastructure/catalogs/FileGaugeCatalog.h>

#include "infrastructure/hardware/g540/G540LPT.h"

#include "infrastructure/process/ProcessLifecycle.h"
#include "infrastructure/process/ProcessRunner.h"

#include <ui/widgets/video/VideoOpenGLWidget.h>
#include "ui/presenters/VideoStreamPresenter.h"

#include <QApplication>

#include "mainwindow.h"
#include "application/interactors/AngleFromVideoInteractor.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/logging/FileLogger.h"
#include "ui/adapters/QtVideoSourceAdapter.h"

static QString appStyle()
{
    return R"QSS(
/* ===== Base ===== */
QMainWindow, QWidget#centralWidget {
    background: #F3F4F6;
    color: #111827;
    font-family: "Segoe UI", "Inter", "Arial";
    font-size: 13px;
}

/* ===== Camera grid (фон между тайлами) ===== */
QWidget#cameraGrid { background: transparent; }

QFrame#cameraTile {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 10px;
}
QFrame#cameraTile QLabel {
    color: #374151;
    font-weight: 600;
}

/* ===== Right panel card ===== */
QFrame#rightPanel {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 12px;
}

/* ===== Segmented control container ===== */
QFrame#segmentedBar {
    background: #F3F4F6;
    border: 1px solid #E5E7EB;
    border-radius: 10px;
}

/* ===== Segment buttons ===== */
QPushButton#segmentBtn {
    background: transparent;
    border: none;
    padding: 8px 14px;
    border-radius: 8px;
    color: #374151;
    font-weight: 600;
}

QPushButton#segmentBtn:hover {
    background: #E5E7EB;
}

QPushButton#segmentBtn[active="true"] {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    color: #111827;
}

/* ===== Page content (optional polish) ===== */
QLabel#pageTitle {
    font-size: 16px;
    font-weight: 700;
    color: #111827;
}
QLabel#hintText {
    color: #6B7280;
}

/* ===== Tabs bar ===== */
QFrame#tabsBar {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 12px;
}

/* Кнопки вкладок */
QPushButton#tabBtn {
    background: transparent;
    border: none;
    padding: 8px 16px;
    border-radius: 8px;
    font-weight: 600;
    color: #374151;
}

QPushButton#tabBtn[active="true"] {
    background: #E5E7EB;
    color: #111827;
}

/* ===== Content cards ===== */
QFrame#contentCard {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 12px;
}

QFrame#cameraTile {
    background-color: #000000;
    border-radius: 12px;
}

/* Таблица */
QTableWidget {
    border: none;
    gridline-color: #E5E7EB;
}

QHeaderView::section {
    background: #F3F4F6;
    border: none;
    border-right: 1px solid #E5E7EB;
    border-bottom: 1px solid #E5E7EB;
    padding: 4px;
    font-weight: 600;
}

)QSS";
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(appStyle());

    using namespace infra::camera;
    using namespace infra::clock;
    using namespace infra::logging;
    using namespace infra::hardware;
    using namespace infra::process;
    using namespace infra::catalogs;
    using namespace ui::presenters;
    using namespace ui::widgets;
    using namespace domain::ports;
    using namespace domain::common;
    using namespace fmt;

    std::unique_ptr<ILogger> console_logger = std::make_unique<FileLogger>("app.log");
    FmtLogger logger(*console_logger);

    ProcessLifecycle lifecycle; // IProcessLifecycle
    ProcessRunner process_runner(lifecycle);

    lifecycle.markForward();

    try {
        FilePrinterCatalog printer_catalog("setup/printers");
        FileDisplacementCatalog file_displacement_catalog("setup/displacements");
        FileGaugeCatalog file_gauge_catalog("setup/gauges");
        logger.info("{}", printer_catalog.list());
        logger.info("{}", file_displacement_catalog.list());
        logger.info("{}", file_gauge_catalog.list());
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // infra::state::InMemoryMeasurementContextStore context_store(*console_logger);
    //


    CameraPorts ports {
        .logger = *console_logger,
        .clock = lifecycle.clock()
    };

    // V4LCameraConfig camera_config { .source = "/dev/video0" };
    // V4LCamera video_stream(ports, camera_config);

    // GStreamerCameraConfig camera_config { .pipe = "v4l2src device=/dev/video0 ! videoconvert ! video/x-raw,format=RGB,width=640,height=480,framerate=30/1 ! appsink name=appsink" };
    // GStreamerCameraConfig camera_config { .pipe = "filesrc location=/media/mint/4CC052E2C052D1B6/Documents/MANOTOM/output.avi ! decodebin ! videorate ! videoconvert ! video/x-raw,format=RGB,width=640,height=480,framerate=30/1 ! appsink name=appsink sync=true" };
    // GStreamerCamera video_stream(ports, camera_config);

    DShowCameraConfig camera_config1 = {
        .index = 0
    };
    DShowCamera video_stream1(ports, camera_config1);

    DShowCameraConfig camera_config2 = {
        .index = 1
    };
    DShowCamera video_stream2(ports, camera_config2);

    infra::calculation::CastAnglemeterPorts cast_anglemeter_ports = {
        .logger = *console_logger,
    };
    infra::calculation::CastAnglemeter cast_anglemeter(cast_anglemeter_ports, {});

    application::interactors::AngleFromVideoInteractorPorts angle_from_video_interactor_ports = {
        .logger = *console_logger,
        .anglemeter = cast_anglemeter
    };
    application::interactors::AngleFromVideoInteractor angle_from_video_interactor(angle_from_video_interactor_ports);
    angle_from_video_interactor.start();

    // video_stream.addSink(angle_from_video_interactor);


    ui::adapters::QtVideoSourceAdapter adapter1(video_stream1);
    ui::adapters::QtVideoSourceAdapter adapter2(video_stream2);

    VideoOpenGLWidget widget1(adapter1);
    widget1.show();

    VideoOpenGLWidget widget2(adapter2);
    widget2.show();

    MainWindow w(video_stream_presenter2);
    w.resize(1100, 700);
    w.show();

    video_stream2.start();
    video_stream1.start();

    return app.exec();
}

