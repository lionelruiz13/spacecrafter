#version 430 core
#pragma debug(on)
#pragma optimize(off)


smooth in vec3 Color;
out vec3 color;

void main() {
    color = Color;
}
