#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth;
uniform int  uNoVideo;
uniform float uCircleDiameterPercent;
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

vec3 applyCircleOverlay(vec3 baseColor, float xPix, float yPix)
{
    if (uCircleDiameterPercent <= 0.0)
    {
        return baseColor;
    }

    float radius = uCircleDiameterPercent * 0.005 * float(uHeight);
    vec2 center = vec2((float(uWidth) - 1.0) * 0.5, (float(uHeight) - 1.0) * 0.5);
    vec2 pos = vec2(xPix, yPix);
    float dist = distance(pos, center);

    if (abs(dist - radius) > 0.75)
    {
        return baseColor;
    }

    float angle = atan(pos.y - center.y, pos.x - center.x);
    if (angle < 0.0)
    {
        angle += 6.28318530718;
    }

    float arc = angle * radius;
    float pattern = mod(floor(arc + 0.5), 11.0);

    return (pattern >= 4.0 && pattern < 7.0) ? uCircleColor2 : uCircleColor1;
}

void main()
{
    if (uNoVideo == 1)
    {
        gl_FragColor = vec4(1.0);
        return;
    }

    float sx = vTex.x * float(uWidth)  - 0.5;
    float sy = vTex.y * float(uHeight) - 0.5;

    if (uFormat == 0)
    {
        vec3 color = texture2D(uTex, vTex).bgr;
        gl_FragColor = vec4(applyCircleOverlay(color, sx, sy), 1.0);
        return;
    }

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

    vec3 color = mix(c0, c1, fy);
    gl_FragColor = vec4(applyCircleOverlay(color, sx, sy), 1.0);
}
