#version 450

layout (location=0) in vec4 position;

layout (binding=0) uniform shadowTrace {
    mat4 shadowMatrix;
};

void main() {
    gl_Position = shadowMatrix * position;
}
