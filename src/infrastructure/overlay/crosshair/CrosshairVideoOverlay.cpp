#include "CrosshairVideoOverlay.h"

using namespace infra::overlay;
using namespace domain::common;

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>

namespace {

struct RGB {
    std::uint8_t r, g, b;
};

struct YUV {
    std::uint8_t y, u, v;
};

inline RGB unpackRGBA(std::uint32_t c) {
    return {
        static_cast<std::uint8_t>((c >> 24) & 0xFF),
        static_cast<std::uint8_t>((c >> 16) & 0xFF),
        static_cast<std::uint8_t>((c >> 8)  & 0xFF)
    };
}

// BT.601
inline YUV rgbToYuv(const RGB& rgb) {
    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;

    float y =  0.299f*r + 0.587f*g + 0.114f*b;
    float u = -0.169f*r - 0.331f*g + 0.500f*b + 128.0f;
    float v =  0.500f*r - 0.419f*g - 0.081f*b + 128.0f;

    auto clamp = [](float v) {
        return static_cast<std::uint8_t>(
            std::clamp(v, 0.0f, 255.0f)
        );
    };

    return { clamp(y), clamp(u), clamp(v) };
}

inline void setPixelRGB24(
    std::uint8_t* data,
    int width,
    int x,
    int y,
    const RGB& color)
{
    std::uint8_t* px = data + (y * width + x) * 3;
    px[0] = color.r;
    px[1] = color.g;
    px[2] = color.b;
}

inline void setPixelYUYV(
    std::uint8_t* data,
    int width,
    int x,
    int y,
    const YUV& color)
{
    // 2 пикселя = 4 байта: Y0 U Y1 V
    int pairIndex = (y * width + (x & ~1));
    std::uint8_t* px = data + pairIndex * 2;

    bool isEven = (x % 2 == 0);

    if (isEven) {
        px[0] = color.y;
        px[1] = color.u;
        px[3] = color.v;
    } else {
        px[2] = color.y;
        px[1] = color.u;
        px[3] = color.v;
    }
}

template<typename SetPixelFn, typename ColorType>
void drawCircleImpl(
    std::uint8_t* data,
    int width,
    int height,
    float radiusNorm,
    const ColorType& c1,
    const ColorType& c2,
    SetPixelFn setPixel)
{
    if (width <= 0 || height <= 0)
        return;

    const int cx = width  / 2;
    const int cy = height / 2;

    const float r_norm = std::clamp(radiusNorm, 0.0f, 1.0f);
    const int radius = static_cast<int>(
        r_norm * (std::min(width, height) / 2.0f)
    );

    if (radius <= 0)
        return;

    constexpr int segment = 4;
    constexpr int period  = 12;

    int patternIndex = 0;

    auto nextColor = [&](const ColorType& a,
                         const ColorType& b) -> const ColorType&
    {
        int idx = patternIndex % period;
        ++patternIndex;

        if (idx < segment) return a;
        if (idx < 2*segment) return b;
        return a;
    };

    int x = radius;
    int y = 0;
    int decision = 1 - x;

    auto plot8 = [&](int px, int py)
    {
        auto plot = [&](int xx, int yy)
        {
            if (xx >= 0 && xx < width &&
                yy >= 0 && yy < height)
            {
                const auto& col = nextColor(c1, c2);
                setPixel(data, width, xx, yy, col);
            }
        };

        plot(cx + px, cy + py);
        plot(cx - px, cy + py);
        plot(cx + px, cy - py);
        plot(cx - px, cy - py);
        plot(cx + py, cy + px);
        plot(cx - py, cy + px);
        plot(cx + py, cy - px);
        plot(cx - py, cy - px);
    };

    while (y <= x)
    {
        plot8(x, y);

        y++;

        if (decision <= 0)
        {
            decision += 2*y + 1;
        }
        else
        {
            x--;
            decision += 2*(y - x) + 1;
        }
    }
}

inline void drawRGB24(
    std::uint8_t* data,
    int width,
    int height,
    float radiusNorm,
    std::uint32_t color1,
    std::uint32_t color2)
{
    RGB c1 = unpackRGBA(color1);
    RGB c2 = unpackRGBA(color2);

    drawCircleImpl(
        data,
        width,
        height,
        radiusNorm,
        c1,
        c2,
        [](auto* d, int w, int x, int y, const RGB& c) {
            setPixelRGB24(d, w, x, y, c);
        });
}

inline void drawYUYV(
    std::uint8_t* data,
    int width,
    int height,
    float radiusNorm,
    std::uint32_t color1,
    std::uint32_t color2)
{
    RGB rgb1 = unpackRGBA(color1);
    RGB rgb2 = unpackRGBA(color2);

    YUV c1 = rgbToYuv(rgb1);
    YUV c2 = rgbToYuv(rgb2);

    drawCircleImpl(
        data,
        width,
        height,
        radiusNorm,
        c1,
        c2,
        [](auto* d, int w, int x, int y, const YUV& c) {
            setPixelYUYV(d, w, x, y, c);
        });
}

} // namespace overlay::crosshair

CrosshairVideoOverlay::CrosshairVideoOverlay(CrosshairVideoOverlayConfig config)
    : config_(config)
{
}

VideoFramePacket CrosshairVideoOverlay::draw(VideoFramePacket packet) const
{
    if (!config_.visible)
        return packet;

    if (!packet.frame)
        return packet;

    auto* frame = packet.frame.get();

    std::uint8_t* data = frame->buffer.data;
    const int width  = frame->width;
    const int height = frame->height;

    if (!data || width <= 0 || height <= 0)
        return packet;

    switch (frame->format)
    {
        case PixelFormat::RGB24:
        {
            drawRGB24(
                data,
                width,
                height,
                config_.radius,
                config_.color1,
                config_.color2
            );
            break;
        }

        case PixelFormat::YUYV:
        {
            drawYUYV(
                data,
                width,
                height,
                config_.radius,
                config_.color1,
                config_.color2
            );
            break;
        }

        default:
            // неподдерживаемый формат — ничего не делаем
            break;
    }

    return packet;
}
