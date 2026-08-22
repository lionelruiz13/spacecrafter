//
// black hole photon ring and shadow
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout(location=0) in vec2 Center;
layout(location=1) in vec2 TexCoord;

layout(binding=1) uniform ubo {
    vec4 PhotonColorAndEventRadius;
    vec4 LensColorAndStrength;
    vec4 Controls;
};

layout (constant_id=0) const float viewportY = 1024;

layout(location=0) out vec4 FragColor;

float ring(float r, float center, float width)
{
    float d = abs(r - center) / width;
    return exp(-d * d);
}

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

void main(void)
{
    float EventRadius = PhotonColorAndEventRadius.a;
    vec2 pixel = vec2(gl_FragCoord.x - Center.x, (viewportY - gl_FragCoord.y) - Center.y);
    pixel.x *= 0.92;
    if (length(pixel) > EventRadius)
        discard;
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
