// dso3d

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 4) in float inTexture; //ce n'est pas une vrai texture mais un indice de texture
layout (location = 5) in float inScale;

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
