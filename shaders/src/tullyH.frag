// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=2, set=1) uniform sampler2D texunit0;

layout (binding=1, set=1) uniform ubo {
	float fader;
};
layout (location=0) out vec4 FragColor;

layout (location=0) in Interpolators
{
	vec2 TexCoord;
	float intensity;
} interData;

void main(void)
{
	vec4 textureColor;
		textureColor = texture(texunit0, interData.TexCoord);
		textureColor.a *= fader;
		if (textureColor.a<0.01)
			discard;
	FragColor = textureColor;
}
