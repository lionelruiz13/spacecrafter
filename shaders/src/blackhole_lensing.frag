// Global gravitational lensing of the rendered scene
#version 420

layout(binding=0) uniform sampler2D Scene;
layout(binding=1) uniform LensData {
    vec4 CenterRadiusStrength;
    vec4 ViewportActive;
};

layout(location=0) in vec2 TexCoord;
layout(location=0) out vec4 FragColor;

void main()
{
    vec2 viewport = ViewportActive.xy;
    bool lensEnabled = ViewportActive.z > 0.5;
    bool distortionEnabled = ViewportActive.w > 0.5;
    if (!lensEnabled) {
        FragColor = texture(Scene, TexCoord);
        return;
    }

    vec2 center = CenterRadiusStrength.xy;
    float shadowRadius = max(CenterRadiusStrength.z, 1.0);
    float strength = max(CenterRadiusStrength.w, 0.0);
    vec2 pixel = gl_FragCoord.xy;
    vec2 offset = pixel - center;
    float radius = length(offset);

    if (!distortionEnabled) {
        FragColor = texture(Scene, TexCoord);
        return;
    }

    if (radius < shadowRadius * 0.965) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float einsteinRadius = shadowRadius * (1.28 + 0.46 * strength);
    float influenceRadius = einsteinRadius * (4.5 + 1.5 * strength);
    float distortionFade = 1.0 - smoothstep(influenceRadius * 0.72, influenceRadius, radius);
    float deflection = (einsteinRadius * einsteinRadius) / max(radius * radius, 1.0);
    vec2 sourcePixel = center + offset * (1.0 - deflection * distortionFade);
    vec2 sourceUv = clamp(sourcePixel / viewport, vec2(0.0), vec2(1.0));

    vec4 lensed = texture(Scene, sourceUv);
    float magnification = 1.0 + 0.14 * exp(-pow((radius - einsteinRadius) / max(shadowRadius * 0.12, 1.0), 2.0));
    FragColor = vec4(lensed.rgb * magnification, lensed.a);
}
