#ifndef CLEANGRADUATOR_QTVIDEOOPENGLWIDGET_H
#define CLEANGRADUATOR_QTVIDEOOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_1>
#include <QOpenGLShaderProgram>

#include <memory>
#include <mutex>

#include "domain/core/video/PixelFormat.h"
#include "viewmodels/Observable.h"

namespace mvvm {
    class VideoSourceViewModel;
}

namespace domain::common {
    struct VideoFrame;
    using VideoFramePtr = std::shared_ptr<VideoFrame>;
}

namespace ui::widgets {

    class QtVideoSourceWidget final
        : public QOpenGLWidget
        , protected QOpenGLFunctions_2_1
    {
        Q_OBJECT
    public:
        explicit QtVideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent = nullptr);

    private slots:
        void setVideoFrame(domain::common::VideoFramePtr frame);

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

    private:
        void initShader();
        void initTextureIfNeeded(const domain::common::VideoFrame& frame);
        void uploadFramePBO(const domain::common::VideoFrame& frame);
        void drawQuad(bool noVideo);

    private:
        std::mutex mutex_;
        domain::common::VideoFramePtr current_frame_;
        mvvm::Observable<domain::common::VideoFramePtr>::Subscription frame_sub_;
        mvvm::Observable<bool>::Subscription is_opened_sub_;

        bool is_source_opened_{false};

        GLuint texture_{0};
        GLuint pbo_[2]{0, 0};
        int    pboIndex_{0};

        bool   textureInitialized_{false};
        int    texWidth_{0};
        int    texHeight_{0};

        QOpenGLShaderProgram program_;
        bool shaderInited_{false};

        domain::common::PixelFormat currentFormat_{domain::common::PixelFormat::RGB24};
        int frameWidth_{0};
        int frameHeight_{0};

        GLenum glUploadFormat_{GL_RGB};

    };

}

#endif //CLEANGRADUATOR_QTVIDEOOPENGLWIDGET_H