// dso3d

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 4) in float inTexture; //ce n'est pas une vrai texture mais un indice de texture
layout (location = 5) in float inScale;

layout (location=0) out float scale;
layout (location=1) out float texture;

void main(void)
{
	gl_Position = vec4(inPosition,1.0);

	scale= inScale;
	texture = inTexture;
}
