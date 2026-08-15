//
// black hole accretion disk
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=0) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 clipping_fov;
    float RingScale;
    float fadingFactor;
};

#include <custom_project.glsl>

layout (location=0) in vec2 Position2D;
layout (location=1) in float RadialCoord;
layout (location=2) in float AngleCoord;

layout (location=0) out vec2 TexCoord;
layout (location=1) out float PlanetHalfAngle;
layout (location=2) out float Separation;
layout (location=3) out float SeparationAngle;
layout (location=4) out float NdotL;
layout (location=5) out float fading;

void main()
{
    vec3 position = vec3(Position2D.x, Position2D.y, 0.0) * RingScale;
    vec4 outPos = custom_project(position, ModelViewMatrix, clipping_fov);
    TexCoord = vec2(RadialCoord, AngleCoord);
    PlanetHalfAngle = 0.0;
    Separation = 0.0;
    SeparationAngle = 1.0;
    NdotL = 1.0;
    fading = min(1.0, (outPos.z * (clipping_fov.y - clipping_fov.x) + clipping_fov.x) * clipping_fov.z * fadingFactor);
    gl_Position = outPos;
}
