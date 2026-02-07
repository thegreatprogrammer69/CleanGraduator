#ifndef CLEANGRADUATOR_VIDEOOPENGLWIDGET_H
#define CLEANGRADUATOR_VIDEOOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_2_1>

class VideoStreamPresenter;
struct VideoFrame;

class VideoOpenGLWidget final
    : public QOpenGLWidget
    , protected QOpenGLFunctions_2_1
{
    Q_OBJECT
public:
    explicit VideoOpenGLWidget(
        VideoStreamPresenter& video_stream_presenter,
        QWidget* parent = nullptr
    );

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    void initTextureIfNeeded(const VideoFrame& frame);
    void uploadFramePBO(const VideoFrame& frame);
    void drawQuad();

private slots:
    void onVideoFrameReady();


private:
    VideoStreamPresenter& video_stream_presenter_;

    GLuint texture_{0};
    GLuint pbo_[2]{0, 0};
    int    pboIndex_{0};

    bool   textureInitialized_{false};
    int    texWidth_{0};
    int    texHeight_{0};
};

#endif //CLEANGRADUATOR_VIDEOOPENGLWIDGET_H