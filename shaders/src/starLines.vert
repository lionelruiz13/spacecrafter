//
// starLines
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec3 position;

void main()
{
	gl_Position = vec4(position,1.0);
}
