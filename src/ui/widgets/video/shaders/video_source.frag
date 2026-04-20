#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth;
uniform int  uNoVideo;
uniform vec2 uViewportSize;
uniform float uCircleDiameterPx;
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

vec4 circleOverlayColor(vec2 fragCoord)
{
    if (uNoVideo == 1)
    {
        return vec4(0.0);
    }

    if (uCircleDiameterPx <= 0.0 || uViewportSize.y <= 0.0)
    {
        return vec4(0.0);
    }

    vec2 center = 0.5 * uViewportSize;
    vec2 delta = fragCoord - center;
    float radius = 0.5 * uCircleDiameterPx;
    float dist = length(delta);
    float ringDelta = abs(dist - radius);
    float thickness = 1.0;

    if (ringDelta > thickness)
    {
        return vec4(0.0);
    }

    float angle = atan(delta.y, delta.x);
    if (angle < 0.0)
    {
        angle += 6.28318530718;
    }

    float arc = angle * max(radius, 1.0);
    float pattern = mod(floor(arc), 11.0);

    vec4 color = (pattern < 4.0 || pattern >= 7.0) ? uCircleColor1 : uCircleColor2;
    float alpha = color.a * smoothstep(thickness, 0.0, ringDelta);

    return vec4(color.rgb, alpha);
}

void main()
{
    vec4 baseColor;

    if (uNoVideo == 1)
    {
        baseColor = vec4(240.0 / 255.0, 240.0 / 255.0, 240.0 / 255.0, 1.0);
    }
    else if (uFormat == 0)
    {
        baseColor = vec4(texture2D(uTex, vTex).bgr, 1.0);
    }
    else
    {
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

        baseColor = vec4(mix(c0, c1, fy), 1.0);
    }

    vec4 overlay = circleOverlayColor(gl_FragCoord.xy);
    gl_FragColor = vec4(mix(baseColor.rgb, overlay.rgb, overlay.a), 1.0);
}
