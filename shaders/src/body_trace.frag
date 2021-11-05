//
//	body_trace
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (push_constant) uniform uColor {layout (offset=64) vec3 Color;};
 
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = vec4(Color, 1.0);
}

