//
// big star
//
#version 420

layout (location=0) in vec3 position;

layout (location=0) out vec3 fPosition;
layout (location=1) flat out float fragTime;

layout (binding=0, set=1) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 clipping_fov;
    float radius;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

void main()
{
    fragTime = time;
    fPosition = position;
    gl_Position = custom_project(position * radius, ModelViewMatrix, clipping_fov);
}
