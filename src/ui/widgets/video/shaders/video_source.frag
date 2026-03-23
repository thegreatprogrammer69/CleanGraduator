#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth;
uniform int  uNoVideo;
uniform int  uCircleDiameterPercent;
uniform vec3 uCircleColor1;
uniform vec3 uCircleColor2;

vec3 yuvToRgb601Limited(float y, float u, float v)
{
    y = 1.16438356 * (y - 16.0/255.0);
    u -= 0.5;
    v -= 0.5;

    return clamp(vec3(
        y + 1.59602678 * v,
        y - 0.39176229 * u - 0.81296765 * v,
        y + 2.01723214 * u
    ), 0.0, 1.0);
}

vec3 sampleYuyv(float xPix, float yPix)
{
    xPix = clamp(xPix, 0.0, float(uWidth  - 1));
    yPix = clamp(yPix, 0.0, float(uHeight - 1));

    float pair = floor(xPix * 0.5);
    float odd  = mod(floor(xPix), 2.0);

    vec2 uv = vec2(
        (pair + 0.5) / float(uPackedWidth),
        (floor(yPix) + 0.5) / float(uHeight)
    );

    vec4 yuyv = texture2D(uTex, uv);

    float Y = (odd < 0.5) ? yuyv.r : yuyv.b;

    return yuvToRgb601Limited(Y, yuyv.g, yuyv.a);
}

vec3 sampleVideoColor()
{
    if (uFormat == 0)
    {
        return texture2D(uTex, vTex).bgr;
    }

    float sx = vTex.x * float(uWidth)  - 0.5;
    float sy = vTex.y * float(uHeight) - 0.5;

    float x0 = floor(sx);
    float y0 = floor(sy);

    float fx = fract(sx);
    float fy = fract(sy);

    vec3 c00 = sampleYuyv(x0,     y0);
    vec3 c10 = sampleYuyv(x0 + 1, y0);
    vec3 c01 = sampleYuyv(x0,     y0 + 1);
    vec3 c11 = sampleYuyv(x0 + 1, y0 + 1);

    vec3 c0 = mix(c00, c10, fx);
    vec3 c1 = mix(c01, c11, fx);

    return mix(c0, c1, fy);
}

bool isCirclePixel(vec2 pixel, float radius, out vec3 color)
{
    if (uCircleDiameterPercent <= 0)
    {
        return false;
    }

    vec2 center = vec2(float(uWidth), float(uHeight)) * 0.5;
    vec2 delta = pixel - center;
    float distanceToCenter = length(delta);

    if (abs(distanceToCenter - radius) > 0.5)
    {
        return false;
    }

    float angle = atan(delta.y, delta.x);
    if (angle < 0.0)
    {
        angle += 6.28318530718;
    }

    float arcLength = angle * radius;
    float pattern = mod(floor(arcLength), 11.0);

    color = (pattern < 4.0 || pattern >= 7.0)
        ? uCircleColor1
        : uCircleColor2;

    return true;
}

void main()
{
    if (uNoVideo == 1)
    {
        gl_FragColor = vec4(1.0);
        return;
    }

    vec3 color = sampleVideoColor();

    float radius = float(uHeight) * (float(uCircleDiameterPercent) / 100.0) * 0.5;
    vec2 pixel = vec2(vTex.x * float(uWidth), vTex.y * float(uHeight));

    vec3 circleColor;
    if (isCirclePixel(pixel, radius, circleColor))
    {
        color = circleColor;
    }

    gl_FragColor = vec4(color, 1.0);
}
