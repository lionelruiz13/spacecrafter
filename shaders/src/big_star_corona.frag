//
// big star halo
//
#version 420

layout (location=0) in vec3 fPosition;
layout (location=1) in float time;

layout (location=0) out vec4 fragColor;

layout (constant_id=0) const float frequency = 1.5;

#include <noise.glsl>

void main()
{
    float t = time - length(fPosition);

    // Offset normal with noise
    float ox = snoise(vec4(fPosition, t) * frequency);
    float oy = snoise(vec4((fPosition + 2000.0), t) * frequency);
    float oz = snoise(vec4((fPosition + 4000.0), t) * frequency);
    // Store offsetVec since we want to use it twice.
    vec3 offsetVec = vec3(ox, oy, oz) * 0.1;

    // Get the distance vector from the center
    vec3 nDistVec = normalize(fPosition + offsetVec);

    // Get noise with normalized position to offset the original position
    vec3 position = fPosition + noise(vec4(nDistVec, t), 5, 2.0, 0.7) * 0.1;

    // Calculate brightness based on distance
    float dist = length(position + offsetVec) * 3.0;
    float brightness = (1.0 / (dist * dist) - 0.1) * 0.7;
    fragColor = vec4(brightness, brightness, brightness, 1.);
}
