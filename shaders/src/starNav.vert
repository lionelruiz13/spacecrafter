//
// starNav
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec3 position;
layout (location=3)in vec3 color;
layout (location=4)in float mag;


out vertexData
{
	float mag;
	vec3 color;
} vertexOut;

void main()
{
	gl_Position = vec4(position,1.0);
	vertexOut.color = color;
	vertexOut.mag = mag;
}
