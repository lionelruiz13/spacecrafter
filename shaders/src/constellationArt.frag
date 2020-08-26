//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;
uniform float Intensity;
uniform vec3 Color;

in G2F
{
	vec2 TexCoord;
} g2f;

out vec4 FragColor;

void main(void)
{
	vec3 textureColor = vec3(texture(mapTexture,g2f.TexCoord)).rgb;

	textureColor.r *= Color.r;
	textureColor.g *= Color.g;
	textureColor.b *= Color.b;
	
	textureColor *= Intensity;
	FragColor = vec4(textureColor, 1.0);
}