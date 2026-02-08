// #include <infrastructure/camera/windows/dshow/DShowCameraStream.h>
#include <infrastructure/camera/linux/v4l/V4LCamera.h>
#include <infrastructure/logging/ConsoleLogger.h>
#include <infrastructure/clock/SessionClock.h>

#include <infrastructure/catalogs/FilePrinterCatalog.h>
#include <infrastructure/catalogs/FileDisplacementCatalog.h>
#include <infrastructure/catalogs/FileGaugeCatalog.h>
#include <infrastructure/state/InMemoryMeasurementContextStore.h>


#include "infrastructure/hardware/g540/G540LPT.h"

#include "infrastructure/process/ProcessLifecycle.h"
#include "infrastructure/process/ProcessRunner.h"

#include <ui/widgets/video/VideoOpenGLWidget.h>
#include "ui/presenters/VideoStreamPresenter.h"

#include <QApplication>

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

    std::unique_ptr<ILogger> console_logger = std::make_unique<ConsoleLogger>();
    FmtLogger logger(*console_logger);

    ProcessLifecycle lifecycle; // IProcessLifecycle
    ProcessRunner process_runner(lifecycle);

    FilePrinterCatalog printer_catalog("setup/printers");
    FileDisplacementCatalog file_displacement_catalog("setup/displacements");
    FileGaugeCatalog file_gauge_catalog("setup/gauges");
    MeasurementSetupQuery query(printer_catalog, file_gauge_catalog, file_displacement_catalog);

    infra::state::InMemoryMeasurementContextStore context_store(*console_logger);

    logger.info("{}", query.availableDisplacements());
    logger.info("{}", query.availableGauges());
    logger.info("{}", query.availablePrinters());

    CameraPorts ports {
        .logger = *console_logger,
        .clock = lifecycle.clock()
    };

    CameraConfig camera_config { .source = "/dev/video0" };
    V4LCamera video_stream(ports, camera_config);

    VideoStreamPresenter video_stream_presenter;
    video_stream.addSink(video_stream_presenter.sink());

    VideoOpenGLWidget widget(video_stream_presenter);
    widget.show();

    video_stream.start();

    return app.exec();
}
