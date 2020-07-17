#version 430 core
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout(location = 0) in vec3 vertexPos;
layout(location = 3) in vec3 color;

uniform mat4 Mat;

smooth out vec3 Color;

void main() {
    gl_Position = Mat * vec4(vertexPos, 1);
    Color = color;
}
