//
//	imageUnified
//
#version 420
#include <imageUnified.glsl>

void main(void)
{
	vec4 tex_color = texture(mapTexture,TexCoord);
	tex_color.a *= fader;
	FragColor = tex_color;
}

