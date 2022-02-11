// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec4 inPosition;
layout (location = 1) in float inTexture;
layout (location = 2) in float inRadius;

//uniform mat4 Mat;

layout (location=0) out vec4 outPosition;
layout (location=1) out float texture;
layout (location=2) out float radius;


void main(void)
{
	outPosition = inPosition;
	radius = inRadius;
	texture = inTexture;
}
