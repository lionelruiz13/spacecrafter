// dso3d

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 inPosition;
layout (location = 4) in float inTexture; //ce n'est pas une vrai texture mais un indice de texture
layout (location = 5) in float inScale;

out V2G
{
	vec3 Position;
	float scale;
	float texture;
} v2g;


void main(void)
{
	v2g.Position = inPosition;
	v2g.scale= inScale;
	v2g.texture = inTexture;
}
