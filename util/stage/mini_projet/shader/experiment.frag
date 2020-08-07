#version 450

layout(location = 0) in vec3 Coloring;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(Coloring, 1.0);
}
