#version 430 core
#pragma debug(on)
#pragma optimize(off)

layout(location = 0) in vec3 vertexPos;
out vec3 color;

void main() {
    color = vec3(1,1,0);
}
