#include "VideoOpenGLWidget.h"

#include <cstring>

#include "../../../domain/core/video/PixelFormat.h"
#include "domain/core/common/VideoBuffer.h"
#include "../../../domain/core/video/VideoFrame.h"
#include "ui/presenters/VideoStreamPresenter.h"

VideoOpenGLWidget::VideoOpenGLWidget(
    VideoStreamPresenter& video_stream_presenter,
    QWidget* parent
)
    : QOpenGLWidget(parent)
    , video_stream_presenter_(video_stream_presenter)
{
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    QObject::connect(&video_stream_presenter_, &VideoStreamPresenter::videoFrameReady,
        this, &VideoOpenGLWidget::onVideoFrameReady);
}

void VideoOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenBuffers(2, pbo_);

    glClearColor(0.f, 0.f, 0.f, 1.f);
}

void VideoOpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void VideoOpenGLWidget::paintGL()
{
    const auto frame = video_stream_presenter_.lastVideoFrame();
    if (!frame)
        return;

    if (frame->format != PixelFormat::RGB24)
        return; // позже добавим YUYV

    initTextureIfNeeded(*frame);
    uploadFramePBO(*frame);
    drawQuad();
}

void VideoOpenGLWidget::initTextureIfNeeded(const VideoFrame& frame)
{
    if (textureInitialized_ &&
        frame.width == texWidth_ &&
        frame.height == texHeight_)
        return;

    texWidth_  = frame.width;
    texHeight_ = frame.height;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        texWidth_,
        texHeight_,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    const size_t size = frame.buffer.size;
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);
        glBufferData(
            GL_PIXEL_UNPACK_BUFFER,
            size,
            nullptr,
            GL_STREAM_DRAW
        );
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    textureInitialized_ = true;
}

void VideoOpenGLWidget::uploadFramePBO(const VideoFrame& frame)
{
    const int next = (pboIndex_ + 1) % 2;

    // 1️⃣ upload previous PBO → texture
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[pboIndex_]);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        texWidth_,
        texHeight_,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    // 2️⃣ map next PBO
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[next]);
    glBufferData(
        GL_PIXEL_UNPACK_BUFFER,
        frame.buffer.size,
        nullptr,
        GL_STREAM_DRAW
    );

    void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    std::memcpy(ptr, frame.buffer.data, frame.buffer.size);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    pboIndex_ = next;
}

void VideoOpenGLWidget::drawQuad()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_);

    glBegin(GL_QUADS);
        glTexCoord2f(0.f, 1.f); glVertex2f(-1.f, -1.f);
        glTexCoord2f(1.f, 1.f); glVertex2f( 1.f, -1.f);
        glTexCoord2f(1.f, 0.f); glVertex2f( 1.f,  1.f);
        glTexCoord2f(0.f, 0.f); glVertex2f(-1.f,  1.f);
    glEnd();
}

void VideoOpenGLWidget::onVideoFrameReady() {
    paintGL();
    update();
}
