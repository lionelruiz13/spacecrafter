//
// blackhole accretion disk
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=0) uniform sampler2D Texture;

layout (binding=2, set=0) uniform BlackHoleVisual {
    vec3 DiskColor;
    float DiskIntensity;
    vec3 PhotonColor;
    float Turbulence;
};

layout (location=0) in vec2 TexCoord;
layout (location=1) in float PlanetHalfAngle;
layout (location=2) in float Separation;
layout (location=3) in float SeparationAngle;
layout (location=4) in float NdotL;
layout (location=5) in float fading;

layout (location=0) out vec4 Color;

float band(float x, float center, float width)
{
    float d = abs(x - center) / width;
    return exp(-d * d);
}

float hash21(vec2 p)
{
    p = fract(p * vec2(234.12, 951.41));
    p += dot(p, p + 71.17);
    return fract(p.x * p.y);
}

void main(void)
{
    float r = clamp(TexCoord.x, 0.0, 1.0);
    float a = TexCoord.y * 6.2831853;
    vec4 density = texture(Texture, vec2(r, 0.5));
    float alpha = density.a;

    float innerGlow = band(r, 0.095, 0.050);
    float midGlow = band(r, 0.28, 0.25);
    float outerGlow = band(r, 0.68, 0.43);
    float clump = 0.5 + 0.5 * sin(a * 11.0 + sin(a * 3.0) * 2.0 + r * 29.0);
    float turbulence = 0.69
        + 0.15 * sin(a * 2.0 + r * 13.0 * Turbulence)
        + 0.10 * sin(a * 5.0 - r * 19.0 * Turbulence)
        + 0.07 * sin(a * 9.0 + r * 31.0 * Turbulence)
        + 0.05 * sin(a * 17.0 + r * 11.0 * Turbulence)
        + (clump - 0.5) * 0.08;
    float softBands = 0.95 + 0.05 * sin(r * 17.0 + turbulence * 2.0);
    float doppler = 0.62 + 0.62 * smoothstep(-0.82, 0.92, cos(a - 0.22));
    float innerCut = smoothstep(0.018, 0.090, r);
    float outerFade = 1.0 - smoothstep(0.88, 1.0, r);
    float shadowSide = 0.86 + 0.18 * smoothstep(-0.85, 0.75, sin(a + 0.18));

    vec3 outerDust = DiskColor * 0.15;
    vec3 midDisk = DiskColor;
    vec3 whiteHot = PhotonColor;
    vec3 color = whiteHot * innerGlow + midDisk * midGlow + outerDust * outerGlow;

    float viewShade = 1.0 + 0.018 * abs(NdotL) + 0.004 * clamp(Separation, -1.0, 1.0);
    float occultHint = 1.0 - 0.006 * smoothstep(0.0, PlanetHalfAngle, SeparationAngle);
    alpha *= clamp(turbulence * softBands * occultHint, 0.22, 1.0) * innerCut * outerFade * fading * shadowSide;
    color *= (0.98 + 1.42 * innerGlow) * viewShade * doppler * DiskIntensity;
    Color = vec4(color * alpha, alpha);
}
