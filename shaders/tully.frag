// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

uniform float fader;
out vec4 FragColor;

subroutine vec3 parametricColor();
subroutine uniform parametricColor useColor;

in Interpolators
{
	vec3 TexColor;
	float intensity;
} interData;

subroutine(parametricColor)
vec3 useCustomColor()
{
	return interData.TexColor;
}

subroutine(parametricColor)
vec3 useWhiteColor()
{
	return vec3(1.0,1.0,1.0);
}

void main(void)
{
	vec4 textureColor = vec4(useColor(), interData.intensity * fader);

	if (textureColor.a<0.01)
		discard;
	FragColor = textureColor;
}
