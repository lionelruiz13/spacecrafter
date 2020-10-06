// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform ubo {
	float fader;
};
layout (location=0) out vec4 FragColor;

layout (location=0) in Interpolators
{
	vec3 TexColor;
	float intensity;
} interData;

void mainCustomColor(void)
{
	vec4 textureColor = vec4(interData.TexColor, interData.intensity * fader);

	if (textureColor.a<0.01)
		discard;
	FragColor = textureColor;
}

void mainWhiteColor(void)
{
	vec4 textureColor = vec4(1.0, 1.0, 1.0, interData.intensity * fader);

	if (textureColor.a<0.01)
		discard;
	FragColor = textureColor;
}
