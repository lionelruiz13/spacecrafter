// person

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding = 1, set = 1) uniform ubo {
	uniform vec3 color;
	uniform float fader;
};

layout (location=0) out vec4 Color;

void main(void)
{
	Color = vec4 (color, fader);
}
