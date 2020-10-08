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

layout (binding=3) uniform uNbPoints{int nbPoints;};
layout (binding=2) uniform uFader {float fader;};

layout (location=0) out float indice;

void main()
{
	gl_Position = vec4(position,1.0);
	indice = (1.0-0.9*segment/nbPoints)*fader;
}
