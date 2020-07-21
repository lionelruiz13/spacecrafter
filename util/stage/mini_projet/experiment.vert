#version 420
#pragma debug(on)
#pragma optimize(off)

layout(location = 0) in vec3 vertexPos;
layout(location = 3) in vec3 color;

out vec3 Color;

void main() {
    gl_Position = vec4(vertexPos, 1.0);
    Color = color;
}
