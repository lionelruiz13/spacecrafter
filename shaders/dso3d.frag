// dso3d

#version 420
#pragma debug(on)
#pragma optimize(off)

uniform float fader;
layout (binding=0) uniform sampler2D texunit0;


out vec4 FragColor;


in Interpolators
{
	vec2 TexCoord;
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
