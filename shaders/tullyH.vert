// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 5) in float inRadius;
layout (location = 1) in float inTexture;

uniform mat4 Mat;

out vertexData
{
	float texture;
	float radius;
} vertexOut;


void main(void)
{
	gl_Position = vec4(inPosition,1.0);

	vertexOut.radius = inRadius;
	vertexOut.texture = inTexture;
}
