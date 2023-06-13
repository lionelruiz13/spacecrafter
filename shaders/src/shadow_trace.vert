#version 450

layout (location=0) in vec3 position;

layout (binding=0) uniform global {
    mat3 matrix;
};

void main()
{
    vec3 tmp = matrix * position;
    gl_Position = vec4(tmp.xy, tmp.z * 0.5 + 0.5, 1);
}
