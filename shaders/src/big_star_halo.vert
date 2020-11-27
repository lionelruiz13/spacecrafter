//
// big star halo
//
#version 420

layout (location=0) in vec4 position;
layout (location=1) in vec3 texCoord; // must be between -1 and 1 included (for x and y)

layout (location=0) out vec3 fPosition;
layout (location=1) flat out float fragTime;

#include <cam_block_only.glsl>

void main()
{
    fragTime = time;
    fPosition = texCoord;
    gl_Position = MVP2D * position;
    gl_Position.w = 1.;
}
