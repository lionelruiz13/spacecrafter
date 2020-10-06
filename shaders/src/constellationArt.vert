//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec2 position;
layout (location=1)in vec2 texCoord;

layout (location=0) out vec2 Position;
layout (location=1) out vec2 TexCoord;

void main()
{
	TexCoord = texCoord;
	Position = position;
}
