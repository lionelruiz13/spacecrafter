//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec2 position;
layout (location=1)in vec2 texCoord;

out V2G {
	vec2 Position;
	vec2 TexCoord;
} v2g;

void main()
{
	v2g.TexCoord = texCoord;
	v2g.Position = position;
}
