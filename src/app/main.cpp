#include <../infrastructure/camera/windows/dshow/DShowCameraStream.h>
#include <infrastructure/logging/ConsoleLogger.h>
#include <infrastructure/clock/SessionClock.h>
#include <ui/widgets/video/VideoOpenGLWidget.h>

#include <QApplication>
#include <QTimer>

#include "ui/presenters/VideoStreamPresenter.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ConsoleLogger logger;
    SessionClock clock;
    VideoStreamPresenter video_stream_presenter;



    CameraPorts ports {
        .logger = logger,
        .clock = clock
    };
    DShowCameraStream video_stream(ports, 0, {0, 1, 2, 3, 4, 5, 6, 7});
    video_stream.addSink(video_stream_presenter.sink());

    VideoOpenGLWidget widget(video_stream_presenter);
    widget.show();

    video_stream.start();

    return app.exec();
}
