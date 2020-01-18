// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in float inTexture;
layout (location = 3) in float inScale;

uniform mat4 Mat;

out vertexData
{
	float scale;
	float texture;
	vec3 color;
} vertexOut;


void main(void)
{
	gl_Position = vec4(inPosition,1.0);

	vertexOut.color = inColor;
	vertexOut.scale= inScale;
	vertexOut.texture = inTexture;
}
