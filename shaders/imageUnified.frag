//
//	imageUnified
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;
//or
layout (binding=1) uniform sampler2D mapTextureU;
layout (binding=2) uniform sampler2D mapTextureV;

#include <convertToRGB.glsl>

uniform float fader;
uniform bool transparency;
uniform bool useRGB;
uniform vec4 noColor;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 tex_color;
	if (useRGB)
		tex_color = texture(mapTexture,TexCoord);
	else
		tex_color = vec4(convertToRGB(mapTexture, mapTextureU, mapTextureV, TexCoord),1);

	tex_color.a *= fader;

	if (transparency) {
		vec3 diffVec = abs(tex_color.rgb-noColor.rgb);
		float delta = noColor.a;
		if ((diffVec.r<delta) && (diffVec.g<delta) && (diffVec.b<delta))
			discard;
	}

	FragColor = tex_color;
}
