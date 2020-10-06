//
//	imageUnified
//
#version 420
#include <imageUnified.glsl>

layout (binding=1) uniform sampler2D mapTextureU;
layout (binding=2) uniform sampler2D mapTextureV;

#include <convertToRGB.glsl>

void main(void)
{
	vec4 tex_color = vec4(convertToRGB(mapTexture, mapTextureU, mapTextureV, TexCoord),1);
	vec3 diffVec = abs(tex_color.rgb-noColor.rgb);
	float delta = noColor.a;
	if ((diffVec.r<delta) && (diffVec.g<delta) && (diffVec.b<delta))
		discard;
	tex_color.a *= fader;
	FragColor = tex_color;
}

