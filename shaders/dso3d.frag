// dso3d

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;

out vec4 FragColor;
uniform float fader;

in G2F
{
	vec2 TexCoord;
} g2f;

void main(void)
{
	vec4 textureColor;
	textureColor = texture(texunit0, g2f.TexCoord);
	textureColor.a *= fader;
	if (textureColor.a<0.01)
		discard;
	FragColor = textureColor;
}
