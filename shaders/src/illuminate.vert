//
// illuminate
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec3 position;
layout (location=1)in vec2 texCoord;
layout (location=3)in vec3 texColor;

layout (location=0) out Data
{
	vec3 position;
    vec2 texCoord;
    vec3 texColor;
} data;


void main()
{
	data.position = position;
	data.texCoord = texCoord;
	data.texColor = texColor;
}
