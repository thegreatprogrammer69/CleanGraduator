// #include <infrastructure/camera/windows/dshow/DShowCameraStream.h>
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


#include "application/interactors/AngleFromVideoInteractor.h"
#include "infrastructure/calculation/angle/CastAnglemeter.h"


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

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

    std::unique_ptr<ILogger> console_logger = std::make_unique<ConsoleLogger>();
    FmtLogger logger(*console_logger);

    ProcessLifecycle lifecycle; // IProcessLifecycle
    ProcessRunner process_runner(lifecycle);

    lifecycle.markForward();

    FilePrinterCatalog printer_catalog("setup/printers");
    FileDisplacementCatalog file_displacement_catalog("setup/displacements");
    FileGaugeCatalog file_gauge_catalog("setup/gauges");

    // infra::state::InMemoryMeasurementContextStore context_store(*console_logger);
    //
    logger.info("{}", printer_catalog.list());
    logger.info("{}", file_displacement_catalog.list());
    logger.info("{}", file_gauge_catalog.list());

    CameraPorts ports {
        .logger = *console_logger,
        .clock = lifecycle.clock()
    };

    // V4LCameraConfig camera_config { .source = "/dev/video0" };
    // V4LCamera video_stream(ports, camera_config);

    // GStreamerCameraConfig camera_config { .pipe = "v4l2src device=/dev/video0 ! videoconvert ! video/x-raw,format=RGB,width=640,height=480,framerate=30/1 ! appsink name=appsink" };
    GStreamerCameraConfig camera_config { .pipe = "filesrc location=/media/mint/4CC052E2C052D1B6/Documents/MANOTOM/output.avi ! decodebin ! videorate ! videoconvert ! video/x-raw,format=RGB,width=640,height=480,framerate=30/1 ! appsink name=appsink sync=true" };
    GStreamerCamera video_stream(ports, camera_config);

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

    video_stream.addSink(angle_from_video_interactor);


    VideoStreamPresenter video_stream_presenter;
    video_stream.addSink(video_stream_presenter.sink());

    VideoOpenGLWidget widget(video_stream_presenter);
    widget.show();

    video_stream.start();
    return app.exec();
}
