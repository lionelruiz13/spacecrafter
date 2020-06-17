//
//	body_trail
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
//~ layout (location=1)in vec3 color;
layout (location=0) in vec3 position;
layout (location=4) in float segment;

uniform int nbPoints;
uniform float fader;

out ValueFader
{
	smooth float indice;
} valueFader;

void main()
{
	gl_Position = vec4(position,1.0);
	valueFader.indice = (1.0-0.9*segment/nbPoints)*fader;
}
