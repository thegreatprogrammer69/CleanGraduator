#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth;
uniform int  uNoVideo;

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

void main()
{
    if (uNoVideo == 1)
    {
        gl_FragColor = vec4(1.0);
        return;
    }

    if (uFormat == 0)
    {
        gl_FragColor = vec4(texture2D(uTex, vTex).bgr, 1.0);
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

    gl_FragColor = vec4(mix(c0, c1, fy), 1.0);
}