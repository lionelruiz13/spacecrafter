// Disk material shared by straight and curved trajectories.
vec4 accretionMaterial(sampler2D densityTexture, float r, float a,
                      vec3 diskColor, vec3 photonColor, float intensity, float turbulenceScale)
{
    float innerGlow = exp(-pow((r - 0.095) / 0.050, 2.0));
    float midGlow = exp(-pow((r - 0.28) / 0.25, 2.0));
    float outerGlow = exp(-pow((r - 0.68) / 0.43, 2.0));
    float clump = 0.5 + 0.5 * sin(a * 11.0 + sin(a * 3.0) * 2.0 + r * 29.0);
    float turbulence = 0.69
        + 0.15 * sin(a * 2.0 + r * 13.0 * turbulenceScale)
        + 0.10 * sin(a * 5.0 - r * 19.0 * turbulenceScale)
        + 0.07 * sin(a * 9.0 + r * 31.0 * turbulenceScale)
        + 0.05 * sin(a * 17.0 + r * 11.0 * turbulenceScale)
        + (clump - 0.5) * 0.08;
    float softBands = 0.95 + 0.05 * sin(r * 17.0 + turbulence * 2.0);
    float doppler = 0.62 + 0.62 * smoothstep(-0.82, 0.92, cos(a - 0.22));
    float shadowSide = 0.86 + 0.18 * smoothstep(-0.85, 0.75, sin(a + 0.18));
    // Explicit LOD is defined even in divergent intersection branches.
    float alpha = textureLod(densityTexture, vec2(r, 0.5), 0.0).a;
    alpha *= clamp(turbulence * softBands * 0.994, 0.22, 1.0)
        * smoothstep(0.018, 0.090, r) * (1.0 - smoothstep(0.88, 1.0, r)) * shadowSide;
    vec3 color = photonColor * innerGlow + diskColor * (midGlow + 0.15 * outerGlow);
    color *= (0.98 + 1.42 * innerGlow) * 1.018 * doppler * intensity;
    return vec4(color * alpha, alpha);
}
