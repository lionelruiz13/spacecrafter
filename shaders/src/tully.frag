// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform ubo {
	float fader;
};
layout (location=0) out vec4 FragColor;

layout (location=0) in vec3 TexColor;
layout (location=1) in float intensity;

void main(void)
{
	vec4 textureColor = vec4(TexColor, intensity * fader);

	if (textureColor.a<0.01)
		discard;
	FragColor = textureColor;
}

