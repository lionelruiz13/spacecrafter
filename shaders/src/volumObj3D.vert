//
// Volumetric 3D object
//
#version 420

layout (set=1, binding=0) uniform ubo {
    mat4 ModelViewMatrix;
    mat4 NormalMatrix; // Inverse rotation and scaling of View matrix, and rotation of Model matrix
    vec3 clipping_fov;
};

#include <cam_block_only.glsl>
#include <custom_project.glsl>

layout (location=0) in vec3 position;

layout (location=0) out vec3 posOut;
layout (location=1) out vec3 texOut;

void main()
{
    texOut = (position + 1) / 2;
    vec3 pos = vec3(ModelViewMatrix * vec4(position, 1.f));
    posOut = pos;
    gl_Position = custom_projectNoMV(lightAberrationPosition(pos, light_aberration_beta.xyz), vec3(clipping_fov.x, clipping_fov.y, M_PI/2));
}
