//
//	imageUnified YUVA with Alpha channel
//
#version 420
#include <imageUnified.glsl>

layout (binding=1) uniform sampler2D mapTextureU;
layout (binding=2) uniform sampler2D mapTextureV;
layout (binding=3) uniform sampler2D mapTextureA; // Texture alpha

#include <convertToRGB.glsl>

void main(void)
{
	vec3 tex_color = convertToRGB(mapTexture, mapTextureU, mapTextureV, TexCoord);

	// Compute final alpha by combining fader and video alpha channel
	float videoAlpha = texture(mapTextureA, TexCoord).r;
	float finalAlpha = fader * videoAlpha;

	FragColor = vec4(tex_color, finalAlpha);
}
