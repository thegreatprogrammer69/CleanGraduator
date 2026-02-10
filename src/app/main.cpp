#include "application/fmt/fmt.h"

#include <infrastructure/camera/windows/dshow/DShowCamera.h>
#include <infrastructure/camera/linux/v4l/V4LCamera.h>
#include "infrastructure/camera/linux/gstreamer/GStreamerCamera.h"

#include <infrastructure/logging/ConsoleLogger.h>

#include <infrastructure/clock/SessionClock.h>

// #include <infrastructure/catalogs/FilePrinterCatalog.h>
// #include <infrastructure/catalogs/FileDisplacementCatalog.h>
// #include <infrastructure/catalogs/FileGaugeCatalog.h>

#include "infrastructure/hardware/g540/G540LPT.h"

#include "infrastructure/process/ProcessLifecycle.h"
#include "infrastructure/process/ProcessRunner.h"

#include <ui/widgets/video/QtVideoSourceWidget.h>
#include <viewmodels/video/VideoSourceViewModel.h>

#include <QApplication>

#include "application/interactors/AngleFromVideoInteractor.h"
#include "application/services/VideoSourceGridService.h"
#include "application/usecases/settings/ApplyCameraGridSettings.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"
#include "infrastructure/logging/FileLogger.h"
#include "infrastructure/settings/QtCameraGridSettingsRepository.h"
#include "ui/widgets/QtMainWindow.h"
#include "viewmodels/MainWindowViewModel.h"
#include "viewmodels/settings/VideoSourceGridSettingsViewModel.h"
#include "viewmodels/video/VideoSourceGridViewModel.h"

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

/* ─────────────────────────────────────────────
   Secondary button: "Открыть все камеры"
   ───────────────────────────────────────────── */
QPushButton#openAllCamerasButton {
    background: transparent;
    border: 1px solid palette(mid);
    border-radius: 4px;
    padding: 4px 10px;
    color: palette(button-text);
}

QPushButton#openAllCamerasButton:hover {
    background: palette(base);
}

QPushButton#openAllCamerasButton:pressed {
    background: palette(midlight);
}


/* ─────────────────────────────────────────────
   Error state for cameras edit
   ───────────────────────────────────────────── */
QLineEdit[error="true"] {
    border: 1px solid #c74a4a;
    background: #fff5f5;
}

QLineEdit[error="true"]:focus {
    border: 1px solid #b53b3b;
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
    using namespace ui::widgets;
    using namespace domain::ports;
    using namespace domain::common;
    using namespace fmt;

    // std::unique_ptr<ILogger> console_logger = std::make_unique<FileLogger>("app.log");
    std::unique_ptr<ILogger> console_logger = std::make_unique<ConsoleLogger>();
    FmtLogger logger(*console_logger);

    ProcessLifecycle lifecycle; // IProcessLifecycle
    ProcessRunner process_runner(lifecycle);

    lifecycle.markForward();

    // try {
    //     FilePrinterCatalog printer_catalog("setup/printers");
    //     FileDisplacementCatalog file_displacement_catalog("setup/displacements");
    //     FileGaugeCatalog file_gauge_catalog("setup/gauges");
    //     logger.info("{}", printer_catalog.list());
    //     logger.info("{}", file_displacement_catalog.list());
    //     logger.info("{}", file_gauge_catalog.list());
    // }
    // catch (const std::exception& e) {
    //     std::cerr << e.what() << std::endl;
    // }

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
    DShowCamera video_stream(ports, camera_config1);

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

    // video_stream1.addSink(angle_from_video_interactor);
    // video_stream2.addSink(angle_from_video_interactor);


    mvvm::VideoSourceViewModel video_source_view_model1(video_stream);
    mvvm::VideoSourceViewModel video_source_view_model2(video_stream);


    QtVideoSourceWidget widget1(video_source_view_model1);
    widget1.show();

    QtVideoSourceWidget widget2(video_source_view_model1);
    widget2.show();

    std::vector<mvvm::VideoSourceGridViewModel::Slot> video_slots {
        {0, 0, video_source_view_model1}, {0, 1, video_source_view_model2},
        {1, 0, video_source_view_model1}, {1, 1, video_source_view_model2},
        {2, 0, video_source_view_model1}, {2, 1, video_source_view_model2},
        {3, 0, video_source_view_model1}, {3, 1, video_source_view_model2},
    };
    mvvm::VideoSourceGridViewModel video_source_grid_view_model(video_slots, 4, 2, 4.0/3.0);


    infrastructure::settings::QtCameraGridSettingsRepository settings_repository("settings_repo.ini");

    using VideoSource = application::services::VideoSourceGridService::VideoSource;
    std::vector<VideoSource> video_sources{
        {.index = 1, .video_source =  video_stream},
        {.index = 2, .video_source =  video_stream},
        {.index = 3, .video_source =  video_stream},
        {.index = 4, .video_source =  video_stream},
        {.index = 5, .video_source =  video_stream},
        {.index = 6, .video_source =  video_stream},
        {.index = 7, .video_source =  video_stream},
        {.index = 8, .video_source =  video_stream},
    };
    application::services::VideoSourceGridService grid_service(video_sources);
    application::usecases::ApplyCameraGridSettingsPorts grid_settings_ports = {
        .logger = *console_logger,
        .settings_repo = settings_repository,
        .crosshair_listener = video_source_view_model1,
    };
    application::usecases::ApplyCameraGridSettingsServices grid_settings_services = {
        .grid_service = grid_service
    };
    application::usecases::ApplyCameraGridSettings grid_settings_use_case(grid_settings_ports, grid_settings_services);
    mvvm::VideoSourceGridSettingsViewModel settings_view_model(grid_settings_use_case);

    mvvm::MainWindowViewModel main_window_view_model(video_source_grid_view_model, settings_view_model);

    ui::QtMainWindow w(main_window_view_model);
    w.resize(1100, 700);
    w.show();

    // std::thread thr1([&video_stream]() {
        // try {
        //     video_stream.open();
        // }
        // catch (const std::exception& e) {
        //     std::cerr << e.what() << std::endl;
        // }
    // });

    // std::thread thr2([&video_stream2]() {
    //     try {
    //         video_stream2.open();
    //     }
    //     catch (const std::exception& e) {
    //         std::cerr << e.what() << std::endl;
    //     }
    // });

    return app.exec();
}

