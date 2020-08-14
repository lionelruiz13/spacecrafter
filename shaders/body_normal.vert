//
// body normal
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
layout (location=2)in vec3 normal;

out vertexData
{
	vec2 texcoord;
	vec3 normal;
	vec3 position;
} vertexVtG;

void main()
{
	vertexVtG.position = position;
	vertexVtG.normal = normal;
	vertexVtG.texcoord = texcoord;
}