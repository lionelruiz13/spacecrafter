//
//	imageViewPort
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;
layout (push_constant) uniform uFader {
	layout (offset=64) float fader;
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec4 textureColor = texture(mapTexture,TexCoord);
	textureColor.a *= fader;
	FragColor = textureColor;
}
