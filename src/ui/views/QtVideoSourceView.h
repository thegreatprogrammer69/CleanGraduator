#ifndef CLEANGRADUATOR_QTVIDEOSOURCEVIEW_H
#define CLEANGRADUATOR_QTVIDEOSOURCEVIEW_H

#include <QWidget>

namespace domain::common {
    struct VideoFrame;
}

namespace ui::adapters {
    class QtVideoSourceAdapter;
}

namespace ui::widgets {
    class QtVideoOpenGLWidget;
}

namespace ui::views {

    class QtVideoSourceView final : public QWidget {
        Q_OBJECT
    public:
        explicit QtVideoSourceView(
            adapters::QtVideoSourceAdapter& video_source_adapter,
            QWidget* parent = nullptr
        );

    private slots:
        void onVideoFrameReady();

    private:
        adapters::QtVideoSourceAdapter& video_source_adapter_;
        widgets::QtVideoOpenGLWidget*   gl_widget_{nullptr};
    };

}

#endif //CLEANGRADUATOR_QTVIDEOSOURCEVIEW_H