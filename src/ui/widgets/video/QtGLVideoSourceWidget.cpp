#include "QtGLVideoSourceWidget.h"

#include <QFile>

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <QVector3D>

#include "domain/core/video/VideoFrame.h"
#include "viewmodels/video/VideoSourceViewModel.h"

using namespace ui;
using namespace domain::common;

namespace {
QVector3D colorToVec3(std::uint32_t color) {
    return QVector3D(
        static_cast<float>((color >> 24) & 0xFFu) / 255.0f,
        static_cast<float>((color >> 16) & 0xFFu) / 255.0f,
        static_cast<float>((color >> 8) & 0xFFu) / 255.0f);
}

QString loadTextResource(const char* path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll());
}
}

QtGLVideoSourceWidget::QtGLVideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget* parent)
    : QOpenGLWidget(parent)
{
    frame_sub_ = model.frame.subscribe([this](const auto& a) {
        setVideoFrame(a.new_value);
    });

    is_opened_sub_ = model.is_opened.subscribe([this](const auto& a) {
        std::lock_guard lock(mutex_);
        is_source_opened_ = a.new_value;
        if (!is_source_opened_) {
            current_frame_.reset();
        }
        update();
    });

    circle_diameter_sub_ = model.circleDiameterPercent().subscribe([this](const auto& a) {
        std::lock_guard lock(mutex_);
        circleDiameterPercent_ = std::clamp(a.new_value, 0, 100);
        update();
    }, false);

    circle_color1_sub_ = model.circleColor1().subscribe([this](const auto& a) {
        std::lock_guard lock(mutex_);
        circleColor1_ = a.new_value;
        update();
    }, false);

    circle_color2_sub_ = model.circleColor2().subscribe([this](const auto& a) {
        std::lock_guard lock(mutex_);
        circleColor2_ = a.new_value;
        update();
    }, false);

    circleDiameterPercent_ = std::clamp(model.circleDiameterPercent().get_copy(), 0, 100);
    circleColor1_ = model.circleColor1().get_copy();
    circleColor2_ = model.circleColor2().get_copy();

    update();
}

void QtGLVideoSourceWidget::setVideoFrame(VideoFramePtr frame) {
    std::lock_guard lock(mutex_);
    current_frame_ = frame;
    update();
}

void QtGLVideoSourceWidget::initializeGL() {
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

void QtGLVideoSourceWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void QtGLVideoSourceWidget::paintGL() {
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

void QtGLVideoSourceWidget::initTextureIfNeeded(const VideoFrame& frame) {
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

void QtGLVideoSourceWidget::uploadFramePBO(const VideoFrame& frame) {
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

void QtGLVideoSourceWidget::drawQuad(bool noVideo) {
    glClear(GL_COLOR_BUFFER_BIT);

    program_.bind();

    program_.setUniformValue("uNoVideo", noVideo ? 1 : 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    program_.setUniformValue("uTex", 0);

    const int fmt = (currentFormat_ == PixelFormat::YUYV) ? 1 : 0;
    program_.setUniformValue("uFormat", fmt);
    program_.setUniformValue("uWidth", std::max(1, frameWidth_));
    program_.setUniformValue("uHeight", std::max(1, frameHeight_));
    program_.setUniformValue("uPackedWidth", std::max(1, texWidth_));
    program_.setUniformValue("uCircleDiameterPercent", std::clamp(circleDiameterPercent_, 0, 100));
    program_.setUniformValue("uCircleColor1", colorToVec3(circleColor1_));
    program_.setUniformValue("uCircleColor2", colorToVec3(circleColor2_));

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

#include <QDir>
void QtGLVideoSourceWidget::initShader() {
    if (shaderInited_) {
        return;
    }

    QDir dir(":/");

    QStringList list = dir.entryList(QDir::Files);
    for (const QString &file : list) {
        qDebug() << file;
    }

    const auto vs = loadTextResource(":/ui/widgets/video/shaders/video_source.vert");
    const auto fs = loadTextResource(":/ui/widgets/video/shaders/video_source.frag");

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
