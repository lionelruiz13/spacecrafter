//
// sky_draw
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec2 position;
layout (location=1)in vec3 color;

layout (location=0) out vec3 Color;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	Color = color;
}
