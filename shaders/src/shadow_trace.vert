#version 450

layout (location=0) in vec3 position;

layout (binding=0) uniform global {
    mat3 matrix;
};

void main()
{
    gl_Position = vec4(matrix * position, 1);
}
