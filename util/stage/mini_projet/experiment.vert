#version 430 core
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout(location = 0) in vec3 vertexPos;

uniform mat4 Mat;

void main() {
    mat4 test = mat4(0.5,0,0,0, 0,0.5,0,0, 0,0,0.5,0, 0,0,0,1);
    gl_Position = Mat * vec4(vertexPos, 1);
}
