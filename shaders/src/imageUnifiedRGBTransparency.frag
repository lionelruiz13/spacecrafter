//
//	imageUnified
//
#version 420
#include <imageUnified.glsl>

void main(void)
{
	vec4 tex_color = texture(mapTexture,TexCoord);
	vec3 diffVec = abs(tex_color.rgb-noColor.rgb);
	float delta = noColor.a;
	if ((diffVec.r<delta) && (diffVec.g<delta) && (diffVec.b<delta))
		discard;
	tex_color.a *= fader;
	FragColor = tex_color;
}

