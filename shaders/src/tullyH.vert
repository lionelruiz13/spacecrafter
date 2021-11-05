// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 1) in float inTexture;
layout (location = 2) in float inRadius;

//uniform mat4 Mat;

layout (location=0) out float texture;
layout (location=1) out float radius;


void main(void)
{
	gl_Position = vec4(inPosition,1.0);

	radius = inRadius;
	texture = inTexture;
}
