//
//	sfontHorizontal
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0)uniform sampler2D mapTexture;
layout (binding=1)uniform sampler2D borderTexture;
uniform vec3 Color;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 textureColor = vec4(texture(borderTexture,TexCoord)).rgba;
	vec4 mapColor = vec4(texture(mapTexture,TexCoord)).rgba;
	textureColor.r = mapColor.r * Color.r;
	textureColor.g = mapColor.g * Color.g;
	textureColor.b = mapColor.b * Color.b;

	FragColor = textureColor;
}
