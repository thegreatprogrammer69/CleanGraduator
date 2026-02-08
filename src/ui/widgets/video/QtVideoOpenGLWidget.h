#ifndef CLEANGRADUATOR_QTVIDEOOPENGLWIDGET_H
#define CLEANGRADUATOR_QTVIDEOOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_1>
#include <QOpenGLShaderProgram>

#include "domain/core/video/PixelFormat.h"

namespace domain::common {
    struct VideoFrame;
    using VideoFramePtr = std::shared_ptr<VideoFrame>;
}

namespace ui::widgets {

    class QtVideoOpenGLWidget final
        : public QOpenGLWidget
        , protected QOpenGLFunctions_2_1
    {
        Q_OBJECT
    public:
        explicit QtVideoOpenGLWidget(QWidget* parent = nullptr);

        void setVideoFrame(domain::common::VideoFramePtr frame);

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

    private:
        void initShader();
        void initTextureIfNeeded(const domain::common::VideoFrame& frame);
        void uploadFramePBO(const domain::common::VideoFrame& frame);
        void drawQuad();

    private:
        domain::common::VideoFramePtr current_frame_;

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