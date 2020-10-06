// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 3) in vec3 inColor;
layout (location = 4) in float inTexture;
layout (location = 5) in float inScale;

//uniform mat4 Mat;

layout (location=0) out vertexData
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
