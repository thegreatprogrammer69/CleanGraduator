#include "QtVideoSourceView.h"
#include "ui/adapters/QtVideoSourceAdapter.h"
#include "ui/widgets/QtVideoOpenGLWidget.h"

#include <QVBoxLayout>

namespace ui::views {

    QtVideoSourceView::QtVideoSourceView(
        adapters::QtVideoSourceAdapter& video_source_adapter,
        QWidget* parent
    )
        : QWidget(parent)
        , video_source_adapter_(video_source_adapter)
    {
        gl_widget_ = new widgets::QtVideoOpenGLWidget(this);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(gl_widget_);

        connect(
            &video_source_adapter_,
            &adapters::QtVideoSourceAdapter::videoFrameReady,
            this,
            &QtVideoSourceView::onVideoFrameReady
        );
    }

    void QtVideoSourceView::onVideoFrameReady() {
        auto frame = video_source_adapter_.lastVideoFrame();
        if (!frame) {
            return;
        }

        gl_widget_->setVideoFrame(frame);
    }

}
