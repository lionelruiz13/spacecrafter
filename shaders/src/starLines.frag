//
// starLines
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform ubo {
	vec3 Color;
	float Fader;
};
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = vec4 (Color, Fader);
}

