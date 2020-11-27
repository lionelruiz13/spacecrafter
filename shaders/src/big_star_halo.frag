//
// big star halo
//
#version 420

layout (location=0) in vec3 fPosition;
layout (location=1) in float time;

layout (location=0) out vec4 fragColor;

void main()
{
    float amplitude = (1.0 - fPosition.x * fPosition.x - fPosition.y * fPosition.y) * 0.6;
    fragColor = vec4(amplitude, amplitude, amplitude, 1.0);
}
