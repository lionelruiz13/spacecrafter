//
//	body_Hints
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (push_constant) uniform uColor {vec4 Color;};
 
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = Color;
}


