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
    vec3 PhotonColor = PhotonColorAndEventRadius.rgb;
    float EventRadius = PhotonColorAndEventRadius.a;
    vec2 pixel = vec2(gl_FragCoord.x - Center.x, (viewportY - gl_FragCoord.y) - Center.y);
    pixel.x *= 0.92;
    float radiusPx = length(pixel);
    float r = radiusPx / max(EventRadius, 0.001);
    float angle = atan(pixel.y, pixel.x);

    float edge = smoothstep(0.90, 1.04, r);
    float photon = ring(r, 1.055, 0.020);

    float frontArc = smoothstep(-0.80, 0.25, -sin(angle)) * (1.0 - smoothstep(0.25, 0.98, sin(angle)));
    float beaming = 0.58 + 0.70 * smoothstep(-0.88, 0.82, cos(angle - 0.28));
    float broken = 0.86 + 0.10 * sin(angle * 5.0 + r * 8.5) + 0.06 * sin(angle * 13.0 - r * 4.0);
    broken += (hash21(vec2(floor(angle * 28.0), floor(r * 18.0))) - 0.5) * 0.08;
    broken = clamp(broken, 0.62, 1.08);

    vec3 color = PhotonColor * photon * 0.72 * beaming * frontArc * broken;
    color *= edge;

    // The event horizon is composed after global lensing. Keeping it in the
    // source image would make the black hole lens its own silhouette.
    float alpha = clamp(photon * 0.60 * frontArc, 0.0, 1.0);

    FragColor = vec4(color, alpha);
}
