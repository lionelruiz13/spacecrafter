#version 450

#define M_PI_2 1.5707963267948966

layout (location=0) in vec3 position;
layout (location=0) out vec3 direction;

layout (binding=0) uniform ubo {
    mat3 ModelViewMatrix;
    vec3 shape; // Inverse of the box scale, unit is sample
    float fov; // fov*M_PI/360
};

#include <custom_project.glsl>

void main()
{
    vec3 pos = position * shape; // Undo scaling
    direction = pos;
    pos = ModelViewMatrix * pos;
    vec4 projected = custom_project2D(vec4(pos, 1.0), mat4(1.0), fov);
    gl_Position = vec4(projected.xy, 0, 1);
}
