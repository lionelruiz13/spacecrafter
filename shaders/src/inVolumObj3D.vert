#version 450

#define M_PI_2 1.5707963267948966

layout (location=0) in vec3 position;
layout (location=0) out vec3 direction;

layout (binding=0) uniform ubo {
    mat3 ModelViewMatrix;
    vec3 shape; // Inverse of the box scale, unit is sample
    float fov; // fov*M_PI/360
};

void main()
{
    vec3 pos = position * shape; // Undo scaling
    direction = pos;
    pos = ModelViewMatrix * pos;
    pos /= sqrt(pos.x*pos.x + pos.y*pos.y) + 1e-30;
    const float f = (atan(pos.z) + M_PI_2) / fov;
    gl_Position = vec4(pos.x * f, pos.y * f, 0, 1);
}
