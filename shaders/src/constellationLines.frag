//
//	CONSTELLATION_LINES
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (location=0) in vec4 Color;
 
layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = Color;
}
