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
	tex_color.a *= fader;
	FragColor = tex_color;
}

