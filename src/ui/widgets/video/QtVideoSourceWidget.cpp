#include "QtVideoSourceWidget.h"
#include <cstring>

#include "domain/core/video/VideoFrame.h"
#include "viewmodels/video/VideoSourceViewModel.h"

using namespace ui::widgets;
using namespace domain::common;

QtVideoSourceWidget::QtVideoSourceWidget(mvvm::VideoSourceViewModel& model, QWidget *parent) : QOpenGLWidget(parent) {
    frame_sub_ = model.frame.subscribe([this] (const auto& a) {
         setVideoFrame(a.new_value);
    });
    crosshair_sub_ = model.crosshair.subscribe([this] (const auto& a) {
        current_crosshair_ = a.new_value;
    });

    repaintTimer_.setInterval(500);
    connect(&repaintTimer_, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
    repaintTimer_.start();
}

void QtVideoSourceWidget::setVideoFrame(VideoFramePtr frame) {
    std::lock_guard lock(mutex_);
    current_frame_ = frame;
    lastFrameTime_ = std::chrono::steady_clock::now();
    update();
}

void QtVideoSourceWidget::initializeGL() {
    initializeOpenGLFunctions();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Фильтры будем выставлять в initTextureIfNeeded() по формату кадра
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenBuffers(2, pbo_);

    initShader();

    glClearColor(0.f, 0.f, 0.f, 1.f);
}


void QtVideoSourceWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void QtVideoSourceWidget::paintGL() {
    if (!shaderInited_) return;

    std::lock_guard lock(mutex_);

    const auto frame = current_frame_;
    if (!frame)
        return;

    if (frame->format != PixelFormat::RGB24 &&
        frame->format != PixelFormat::YUYV)
        return;

    initTextureIfNeeded(*frame);
    uploadFramePBO(*frame);
    drawQuad();
}

void QtVideoSourceWidget::initTextureIfNeeded(const VideoFrame &frame) {
    const bool isYuyv = (frame.format == PixelFormat::YUYV);

    if (isYuyv && (frame.width % 2) != 0)
        return; // YUYV требует чётную ширину (или обработай отдельно)

    const int desiredTexW = isYuyv ? (frame.width / 2) : frame.width;
    const int desiredTexH = frame.height;
    const GLenum desiredUploadFmt = isYuyv ? GL_RGBA : GL_RGB;

    if (textureInitialized_ &&
        desiredTexW == texWidth_ &&
        desiredTexH == texHeight_ &&
        frame.format == currentFormat_)
        return;

    currentFormat_ = frame.format;
    frameWidth_ = frame.width;
    frameHeight_ = frame.height;

    texWidth_ = desiredTexW;
    texHeight_ = desiredTexH;
    glUploadFormat_ = desiredUploadFmt;

    glBindTexture(GL_TEXTURE_2D, texture_);

    // Важно:
    // - Для RGB оставим LINEAR
    // - Для YUYV ставим NEAREST, потому что выборки делаем сами (bilinear в шейдере)
    if (isYuyv) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    const GLenum internalFmt = isYuyv ? GL_RGBA : GL_RGB;

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFmt,
        texWidth_,
        texHeight_,
        0,
        glUploadFormat_,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    const size_t size = frame.buffer.size;
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    textureInitialized_ = true;
}

void QtVideoSourceWidget::uploadFramePBO(const VideoFrame &frame) {
    const int next = (pboIndex_ + 1) % 2;

    // 1) upload previous PBO -> texture
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[pboIndex_]);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0, 0,
        texWidth_,
        texHeight_,
        glUploadFormat_, // GL_RGB или GL_RGBA
        GL_UNSIGNED_BYTE,
        nullptr
    );

    // 2) map next PBO and copy new frame
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_[next]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, frame.buffer.size, nullptr, GL_STREAM_DRAW);

    void *ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        std::memcpy(ptr, frame.buffer.data, frame.buffer.size);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    pboIndex_ = next;
}

void QtVideoSourceWidget::drawQuad() {
    glClear(GL_COLOR_BUFFER_BIT);

    program_.bind();

    // uNoVideo
    const auto now = std::chrono::steady_clock::now();
    const bool noVideo =
        !current_frame_ ||
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastFrameTime_).count() > 500;

    program_.setUniformValue("uNoVideo", noVideo ? 1 : 0);

    // Закругленные края
    const float dpr = devicePixelRatioF();
    program_.setUniformValue("uSize", QVector2D(width() * dpr, height() * dpr));
    program_.setUniformValue("uRadius", 12.0f * dpr);

    // Перекрестие
    program_.setUniformValue("uCrosshairVisible", current_crosshair_.visible);
    program_.setUniformValue("uCrosshairRadius", current_crosshair_.radius);
    program_.setUniformValue("uCrosshairColor1", current_crosshair_.color.color1);
    program_.setUniformValue("uCrosshairColor2", current_crosshair_.color.color2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    program_.setUniformValue("uTex", 0);

    const int fmt = (currentFormat_ == PixelFormat::YUYV) ? 1 : 0;
    program_.setUniformValue("uFormat", fmt);
    program_.setUniformValue("uWidth", frameWidth_);
    program_.setUniformValue("uHeight", frameHeight_);
    program_.setUniformValue("uPackedWidth", texWidth_);

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
    if (shaderInited_) return;

    // Вершинник: берём координаты из fixed pipeline (glBegin/glTexCoord2f)
    static const char *vs = R"(
        #version 120
        varying vec2 vTex;
        void main() {
            vTex = gl_MultiTexCoord0.st;
            gl_Position = gl_Vertex;
        }
    )";

    // Фрагментник: RGB passthrough или YUYV->RGB на GPU.
    // uFormat: 0=RGB, 1=YUYV(packed RGBA: Y0 U Y1 V)
    static const char *fs = R"(
#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth; // YUYV = width/2, RGB = width

uniform int uNoVideo; // 0 = есть видео, 1 = нет видео

uniform vec2  uSize;     // размер виджета в пикселях
uniform float uRadius;   // радиус скругления

// перекрестие / кольцо
uniform int   uCrosshairVisible;
uniform float uCrosshairRadius;   // 0..1
uniform vec3  uCrosshairColor1;
uniform vec3  uCrosshairColor2;


// ------------------------------------------------------------
// Rounded rect
bool insideRoundedRect(vec2 p, vec2 size, float r)
{
    vec2 q = abs(p - size * 0.5) - (size * 0.5 - vec2(r));
    return length(max(q, 0.0)) <= r;
}


// ------------------------------------------------------------
// YUV -> RGB
vec3 yuvToRgb601Limited(float y, float u, float v)
{
    y = 1.16438356 * (y - 16.0/255.0);
    u = u - 0.5;
    v = v - 0.5;

    float r = y + 1.59602678 * v;
    float g = y - 0.39176229 * u - 0.81296765 * v;
    float b = y + 2.01723214 * u;

    return clamp(vec3(r,g,b), 0.0, 1.0);
}

vec3 sampleYuyvNearest(float xPix, float yPix)
{
    xPix = clamp(xPix, 0.0, float(uWidth  - 1));
    yPix = clamp(yPix, 0.0, float(uHeight - 1));

    float x0   = floor(xPix);
    float y0   = floor(yPix);

    float pair = floor(x0 * 0.5);
    float odd  = mod(x0, 2.0);

    float tx = (pair + 0.5) / float(uPackedWidth);
    float ty = (y0   + 0.5) / float(uHeight);

    vec4 yuyv = texture2D(uTex, vec2(tx, ty)); // Y0 U Y1 V

    float Y = mix(yuyv.r, yuyv.b, odd);
    return yuvToRgb601Limited(Y, yuyv.g, yuyv.a);
}


// ------------------------------------------------------------
// Center ring (12 px: 4 c1 → 4 c2 → 4 c1)
vec4 drawCenterRing(vec2 fragPos)
{
    if (uCrosshairVisible == 0 || uCrosshairRadius <= 0.0)
        return vec4(0.0);

    vec2 center = uSize * 0.5;
    float dist  = length(fragPos - center);

    float maxR  = min(uSize.x, uSize.y) * 0.5;
    float ringR = uCrosshairRadius * maxR;

    float halfThickness = 6.0; // 12px total
    float d = abs(dist - ringR);

    if (d > halfThickness)
        return vec4(0.0);

    float t = d + halfThickness; // 0..12

    vec3 color;
    if (t < 4.0)
        color = uCrosshairColor1;
    else if (t < 8.0)
        color = uCrosshairColor2;
    else
        color = uCrosshairColor1;

    return vec4(color, 1.0);
}


// ------------------------------------------------------------
void main()
{
    vec2 fragPos = vTex * uSize;

    if (!insideRoundedRect(fragPos, uSize, uRadius)) {
        gl_FragColor = vec4(243.0/255.0, 244.0/255.0, 246.0/255.0, 1.0);
        return;
    }

    if (uNoVideo == 1) {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    vec4 baseColor;

    if (uFormat == 0) {
        baseColor = vec4(texture2D(uTex, vTex).rgb, 1.0);
    } else {
        float sx = vTex.x * float(uWidth)  - 0.5;
        float sy = vTex.y * float(uHeight) - 0.5;

        float x0 = floor(sx);
        float y0 = floor(sy);

        float fx = fract(sx);
        float fy = fract(sy);

        vec3 c00 = sampleYuyvNearest(x0,     y0);
        vec3 c10 = sampleYuyvNearest(x0 + 1, y0);
        vec3 c01 = sampleYuyvNearest(x0,     y0 + 1);
        vec3 c11 = sampleYuyvNearest(x0 + 1, y0 + 1);

        vec3 c0 = mix(c00, c10, fx);
        vec3 c1 = mix(c01, c11, fx);

        baseColor = vec4(mix(c0, c1, fy), 1.0);
    }

    // overlay кольца
    vec4 ring = drawCenterRing(fragPos);
    if (ring.a > 0.0)
        baseColor.rgb = mix(baseColor.rgb, ring.rgb, ring.a);

    gl_FragColor = baseColor;
}
)";

    program_.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    program_.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
    program_.link();

    shaderInited_ = true;
}
