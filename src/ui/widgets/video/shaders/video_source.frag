#version 120

varying vec2 vTex;

uniform sampler2D uTex;
uniform int  uFormat;
uniform int  uWidth;
uniform int  uHeight;
uniform int  uPackedWidth;

uniform int uNoVideo;

uniform vec2  uSize;
uniform float uRadius;

bool insideRoundedRect(vec2 p, vec2 size, float r)
{
    vec2 q = abs(p - size * 0.5) - (size * 0.5 - vec2(r));
    return length(max(q, 0.0)) <= r;
}

vec3 yuvToRgb601Limited(float y, float u, float v)
{
    y = 1.16438356 * (y - 16.0/255.0);
    u = u - 0.5;
    v = v - 0.5;

    float r = y + 1.59602678 * v;
    float g = y - 0.39176229 * u - 0.81296765 * v;
    float b = y + 2.01723214 * u;

    return clamp(vec3(r, g, b), 0.0, 1.0);
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

    vec4 yuyv = texture2D(uTex, vec2(tx, ty));

    float Y = mix(yuyv.r, yuyv.b, odd);
    return yuvToRgb601Limited(Y, yuyv.g, yuyv.a);
}

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

    gl_FragColor = baseColor;
}
