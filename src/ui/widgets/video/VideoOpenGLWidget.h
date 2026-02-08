#ifndef CLEANGRADUATOR_VIDEOOPENGLWIDGET_H
#define CLEANGRADUATOR_VIDEOOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_1>
#include <QOpenGLShaderProgram>

#include "domain/core/video/PixelFormat.h"

namespace domain::common {
    struct VideoFrame;
}

namespace ui::presenters {
    class VideoStreamPresenter;
}

namespace ui::widgets {

class VideoOpenGLWidget final
    : public QOpenGLWidget
    , protected QOpenGLFunctions_2_1
{
    Q_OBJECT
public:
    explicit VideoOpenGLWidget(
        presenters::VideoStreamPresenter& video_stream_presenter,
        QWidget* parent = nullptr
    );

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void initTextureIfNeeded(const domain::common::VideoFrame& frame);
    void uploadFramePBO(const domain::common::VideoFrame& frame);
    void drawQuad();

private slots:
    void onVideoFrameReady();


private:
    presenters::VideoStreamPresenter& video_stream_presenter_;

    GLuint texture_{0};
    GLuint pbo_[2]{0, 0};
    int    pboIndex_{0};

    bool   textureInitialized_{false};
    int    texWidth_{0};
    int    texHeight_{0};

    void initShader();

    QOpenGLShaderProgram program_;
    bool shaderInited_ = false;

    // чтобы шейдер знал, что сейчас загружено
    domain::common::PixelFormat currentFormat_ = domain::common::PixelFormat::RGB24;
    int frameWidth_  = 0;
    int frameHeight_ = 0;

    GLenum glUploadFormat_ = GL_RGB; // GL_RGB или GL_RGBA
};

}

#endif //CLEANGRADUATOR_VIDEOOPENGLWIDGET_H