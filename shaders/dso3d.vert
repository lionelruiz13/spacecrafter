// dso3d

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 1) in float inTexture;
layout (location = 2) in float inScale;

uniform mat4 Mat;
uniform vec3 camPos;

out vertexData
{
	float scale;
	float texture;
} vertexOut;


void main(void)
{
	gl_Position = vec4(inPosition,1.0);

	vertexOut.scale= inScale;
	vertexOut.texture = inTexture;
}
