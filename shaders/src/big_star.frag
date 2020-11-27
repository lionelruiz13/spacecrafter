//
// big star
//
#version 420

layout (location=0) in vec3 fPosition; // raw model position
layout (location=1) in float time;

layout (location=0) out vec4 fragColor;

layout (binding=1, set=1) uniform ubo {
    vec3 color;
    float radius;
    vec3 cam_view; // normalized view direction to the center of this body
};

layout (constant_id=0) const float s = 0.3;
layout (constant_id=1) const float frequency = 0.00001;

#include <noise.glsl>

void main()
{
    vec4 position = vec4(fPosition, time);
    float n = (noise(position, 4, 40.0, 0.7) + 1.0) * 0.5;

    // Sunspots
    vec4 sPosition = vec4(fPosition * radius * 20000000, time);
    float t1 = snoise(sPosition * frequency) - s;
    float t2 = snoise((sPosition + radius) * frequency) - s;
    float ss = (max(t1, 0.0) * max(t2, 0.0)) * 2.0;

    // Accumulate total noise
    float intensity = n - ss;
    fragColor = vec4(color * intensity + dot(cam_view, fPosition) * 0.5, 1.0);
}
