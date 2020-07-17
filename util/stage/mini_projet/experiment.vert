#version 430 core
#pragma debug(on)
#pragma optimize(off)

layout(location = 0) in vec3 vertexPos;

void main() {
    gl_Position.xyz = vertexPos;
    gl_Position.w = 1.0;
}
