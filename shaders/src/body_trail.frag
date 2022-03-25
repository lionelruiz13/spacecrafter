//
//	body_trail
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (push_constant) uniform uColor {vec3 Color;};

layout (location=0) out vec4 FragColor;

layout (location=0) in float indice;

void main(void)
{
	FragColor = vec4(Color, indice);
}
