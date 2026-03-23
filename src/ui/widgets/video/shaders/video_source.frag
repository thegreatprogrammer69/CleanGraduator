#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth;
uniform int  uNoVideo;
uniform vec2 uViewportSize;
uniform int  uCircleVisible;
uniform float uCircleDiameterPercent;
uniform vec4 uCircleColor1;
uniform vec4 uCircleColor2;

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

vec3 blendOverlay(vec3 baseColor, vec4 overlayColor)
{
    return mix(baseColor, overlayColor.rgb, overlayColor.a);
}

vec3 applyCircleOverlay(vec3 baseColor)
{
    if (uCircleVisible == 0)
    {
        return baseColor;
    }

    float radius = clamp(uCircleDiameterPercent, 0.0, 100.0) * 0.005 * uViewportSize.y;
    if (radius <= 0.0)
    {
        return baseColor;
    }

    vec2 center = uViewportSize * 0.5;
    vec2 delta = gl_FragCoord.xy - center;
    float distanceToCenter = length(delta);

    if (abs(distanceToCenter - radius) > 0.75)
    {
        return baseColor;
    }

    float angle = atan(delta.y, delta.x);
    if (angle < 0.0)
    {
        angle += 6.28318530718;
    }

    float arcPosition = floor(angle * radius + 0.5);
    float patternIndex = mod(arcPosition, 11.0);
    vec4 overlayColor = (patternIndex < 4.0 || patternIndex >= 7.0) ? uCircleColor1 : uCircleColor2;

    return blendOverlay(baseColor, overlayColor);
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

void main()
{
    if (uNoVideo == 1)
    {
        gl_FragColor = vec4(1.0);
        return;
    }

    if (uFormat == 0)
    {
        vec3 baseColor = texture2D(uTex, vTex).bgr;
        gl_FragColor = vec4(applyCircleOverlay(baseColor), 1.0);
        return;
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

    vec3 baseColor = mix(c0, c1, fy);
    gl_FragColor = vec4(applyCircleOverlay(baseColor), 1.0);
}