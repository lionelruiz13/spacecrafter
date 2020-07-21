#version 420
#pragma debug(on)
#pragma optimize(off)


in vec3 Coloring;
out vec3 color;

void main() {
    color = Coloring;
}
