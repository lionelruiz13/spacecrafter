// shadowRing.vert - TEXTURED_ANNULUS word (G8, ShadowService): vertex-less
// strip quad spanning the caster's equatorial plane (z=0), unit = the OUTER
// ring radius (the ShadowCaster radius the silhouette matrix was built with,
// so `matrix` maps these coords to shadow-map NDC exactly like mesh
// positions). The frag reconstructs the radial coordinate from `plane`.
#version 450

layout (binding=0) uniform global {
    mat3 matrix;
};

layout (location=0) out vec2 plane;

const vec2 corners[4] = vec2[4](vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));

void main()
{
    plane = corners[gl_VertexIndex];
    vec3 tmp = matrix * vec3(plane, 0.0);
    gl_Position = vec4(tmp.xy, tmp.z * 0.5 + 0.5, 1.0);
}
