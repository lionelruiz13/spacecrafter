//
// illuminate
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec3 position;
layout (location=1)in vec2 texCoord;
layout (location=2)in vec3 texColor;

layout (location=0) out vec3 dataposition;
layout (location=1) out vec2 datatexCoord;
layout (location=2) out vec3 datatexColor;

void main()
{
	dataposition = position;
	datatexCoord = texCoord;
	datatexColor = texColor;
}
