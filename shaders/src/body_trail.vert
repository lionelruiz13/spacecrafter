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

layout (push_constant) uniform ubo {
	layout (offset=76) int nbPoints;
	float fader;
};

layout (location=0) out float indice;

void main()
{
	gl_Position = vec4(position,1.0);
	indice = (1.0-0.9*gl_VertexIndex/nbPoints)*fader;
}
