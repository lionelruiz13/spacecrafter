//
//	imageViewPort YUVA with Alpha channel
//
#version 420
#pragma debug(on)
#pragma optimize(off)

#include <convertToRGB.glsl>

layout (binding=0) uniform sampler2D mapTexture;  // Y texture (like unified shader)
layout (binding=1) uniform sampler2D mapTextureU;
layout (binding=2) uniform sampler2D mapTextureV;
layout (binding=3) uniform sampler2D mapTextureA; // Texture alpha

layout (push_constant) uniform uFrag {
	layout (offset=76) float fader;
	layout (offset=80) vec4 noColor;  // Add noColor like unified shader
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec3 tex_color = convertToRGB(mapTexture, mapTextureU, mapTextureV, TexCoord);

	// Compute final alpha by combining fader and video alpha channel
	float videoAlpha = texture(mapTextureA, TexCoord).r;
	float finalAlpha = fader * videoAlpha;

	FragColor = vec4(tex_color, finalAlpha);
}