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
uniform vec4 noColor;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

vec4 tex_color;

// utilisation de fonctions spécialisées afin d'utiliser soit une RGB texture soit 3 YUV textures
subroutine void parametricTexture();
subroutine uniform parametricTexture defineTexture;

subroutine(parametricTexture)
void useRGB()
{
	tex_color = texture(mapTexture,TexCoord);
}

subroutine(parametricTexture)
void useYUV()
{
	tex_color = vec4(convertToRGB(mapTexture, mapTextureU, mapTextureV, TexCoord),1);
}


// utilisation de fonctions spécialisées afin d'appliquer une transparence ou non
subroutine void parametricTransparency();
subroutine uniform parametricTransparency defineTransparency;

subroutine(parametricTransparency)
void useTransparency()
{
	vec3 diffVec = abs(tex_color.rgb-noColor.rgb);
	float delta = noColor.a;
	if ((diffVec.r<delta) && (diffVec.g<delta) && (diffVec.b<delta))
		discard;
}

subroutine(parametricTransparency)
void useNoTransparency()
{}

void main(void)
{
	// complète tex_color avec la/les bonne texture
	defineTexture();
	tex_color.a *= fader;

	// applique la transparence au pixel
	defineTransparency();

	FragColor = tex_color;
}
