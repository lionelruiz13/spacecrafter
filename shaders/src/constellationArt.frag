//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D mapTexture;
layout (push_constant) uniform pushC {
	vec3 Color;
	layout (offset=12) float Intensity;
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec3 textureColor = vec3(texture(mapTexture,TexCoord)).rgb;

	textureColor.r *= Color.r;
	textureColor.g *= Color.g;
	textureColor.b *= Color.b;
	
	textureColor *= Intensity;
	FragColor = vec4(textureColor, 1.0);
}
