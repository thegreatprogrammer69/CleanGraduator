#include "QtVideoSourceWidget.h"

#include <QFile>

#include <algorithm>
#include <cstring>

#include "domain/core/video/VideoFrame.h"
#include "viewmodels/video/VideoSourceViewModel.h"

using namespace ui::widgets;
using namespace domain::common;

namespace {
QString loadTextResource(const char* path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll());
}
}

QtVideoSourceWidget::QtVideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent)
    : QOpenGLWidget(parent)
{
    frame_sub_ = model.frame.subscribe([this](const auto& a) {
        setVideoFrame(a.new_value);
    });
    crosshair_sub_ = model.crosshair.subscribe([this](const auto& a) {
        std::lock_guard lock(mutex_);
        current_crosshair_ = a.new_value;
        update();
    });
    is_opened_sub_ = model.is_opened.subscribe([this](const auto& a) {
        std::lock_guard lock(mutex_);
        is_source_opened_ = a.new_value;
        if (!is_source_opened_) {
            current_frame_.reset();
        }
        update();
    });

    update();
}

void QtVideoSourceWidget::setVideoFrame(VideoFramePtr frame) {
    std::lock_guard lock(mutex_);
    current_frame_ = frame;
    update();
}

void QtVideoSourceWidget::initializeGL() {
    initializeOpenGLFunctions();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenBuffers(2, pbo_);

    initShader();

    glClearColor(0.f, 0.f, 0.f, 1.f);

    update();
}

void QtVideoSourceWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void QtVideoSourceWidget::paintGL() {
    if (!shaderInited_) {
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    std::lock_guard lock(mutex_);

    const auto frame = current_frame_;
    const bool hasRenderableFrame = frame && (frame->format == PixelFormat::RGB24 || frame->format == PixelFormat::YUYV);
    if (hasRenderableFrame) {
        initTextureIfNeeded(*frame);
        uploadFramePBO(*frame);
    }

    const bool noVideo = !is_source_opened_ || !hasRenderableFrame;
    drawQuad(noVideo);
}

void QtVideoSourceWidget::initTextureIfNeeded(const VideoFrame& frame) {
    const bool isYuyv = (frame.format == PixelFormat::YUYV);

    if (isYuyv && (frame.width % 2) != 0)
        return;

    const int desiredTexW = isYuyv ? (frame.width / 2) : frame.width;
    const int desiredTexH = frame.height;
    const GLenum desiredUploadFmt = isYuyv ? GL_RGBA : GL_RGB;

    if (textureInitialized_ && desiredTexW == texWidth_ && desiredTexH == texHeight_ && frame.format == currentFormat_)
        return;

    currentFormat_ = frame.format;
    frameWidth_ = frame.width;
    frameHeight_ = frame.height;

    texWidth_ = desiredTexW;
    texHeight_ = desiredTexH;
    glUploadFormat_ = desiredUploadFmt;

    glBindTexture(GL_TEXTURE_2D, texture_);

    if (isYuyv) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    const GLenum internalFmt = isYuyv ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, texWidth_, texHeight_, 0, glUploadFormat_, GL_UNSIGNED_BYTE, nullptr);

    const size_t size = frame.buffer.size;
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    textureInitialized_ = true;
}

void QtVideoSourceWidget::uploadFramePBO(const VideoFrame& frame) {
    const int next = (pboIndex_ + 1) % 2;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[pboIndex_]);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth_, texHeight_, glUploadFormat_, GL_UNSIGNED_BYTE, nullptr);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[next]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, frame.buffer.size, nullptr, GL_STREAM_DRAW);

    void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        std::memcpy(ptr, frame.buffer.data, frame.buffer.size);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    pboIndex_ = next;
}

void QtVideoSourceWidget::drawQuad(bool noVideo) {
    glClear(GL_COLOR_BUFFER_BIT);

    program_.bind();

    program_.setUniformValue("uNoVideo", noVideo ? 1 : 0);

    const float dpr = devicePixelRatioF();
    program_.setUniformValue("uSize", QVector2D(width() * dpr, height() * dpr));
    program_.setUniformValue("uRadius", 12.0f * dpr);

    program_.setUniformValue("uCrosshairVisible", current_crosshair_.visible);
    program_.setUniformValue("uCrosshairRadius", current_crosshair_.radius);
    program_.setUniformValue("uCrosshairColor1", current_crosshair_.color.color1);
    program_.setUniformValue("uCrosshairColor2", current_crosshair_.color.color2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    program_.setUniformValue("uTex", 0);

    const int fmt = (currentFormat_ == PixelFormat::YUYV) ? 1 : 0;
    program_.setUniformValue("uFormat", fmt);
    program_.setUniformValue("uWidth", std::max(1, frameWidth_));
    program_.setUniformValue("uHeight", std::max(1, frameHeight_));
    program_.setUniformValue("uPackedWidth", std::max(1, texWidth_));

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 1.f);
    glVertex2f(-1.f, -1.f);
    glTexCoord2f(1.f, 1.f);
    glVertex2f(1.f, -1.f);
    glTexCoord2f(1.f, 0.f);
    glVertex2f(1.f, 1.f);
    glTexCoord2f(0.f, 0.f);
    glVertex2f(-1.f, 1.f);
    glEnd();

    program_.release();
}

void QtVideoSourceWidget::initShader() {
    if (shaderInited_) {
        return;
    }

    const auto vs = loadTextResource(":/ui/shaders/video_source.vert");
    const auto fs = loadTextResource(":/ui/shaders/video_source.frag");

    if (vs.isEmpty() || fs.isEmpty()) {
        return;
    }

    if (!program_.addShaderFromSourceCode(QOpenGLShader::Vertex, vs)) {
        return;
    }

    if (!program_.addShaderFromSourceCode(QOpenGLShader::Fragment, fs)) {
        return;
    }

    if (!program_.link()) {
        return;
    }

    shaderInited_ = true;
}
