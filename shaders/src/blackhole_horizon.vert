// Event-horizon mesh used when post-process lensing is disabled.
#version 420

layout (binding=0, set=0) uniform ubo {
    mat4 ModelViewMatrix;
    vec3 clipping_fov;
    float HorizonRadius;
};

#include <cam_block.glsl>
#include <custom_project.glsl>

layout (location=0) in vec3 Position;

void main()
{
    gl_Position = custom_projectAberrated(Position * HorizonRadius, ModelViewMatrix, clipping_fov, light_aberration_beta.xyz);
}
