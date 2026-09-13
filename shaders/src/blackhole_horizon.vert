// Event-horizon mesh used when post-process lensing is disabled.
#version 420

layout (binding=0, set=0) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 clipping_fov;
    float HorizonRadius;
};

#include <custom_project.glsl>

layout (location=0) in vec3 Position;

void main()
{
    gl_Position = custom_project(Position * HorizonRadius, ModelViewMatrix, clipping_fov);
}
