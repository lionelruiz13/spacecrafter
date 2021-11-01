//
// starNav
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec3 color;
layout (location=2)in float mag;

layout (location=0) out float magOut;
layout (location=1) out vec3 colorOut;

void main()
{
	gl_Position = vec4(position,1.0);
	colorOut = color;
	magOut = mag;
}
