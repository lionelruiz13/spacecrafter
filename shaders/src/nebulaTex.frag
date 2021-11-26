// nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (set=1, binding=0) uniform sampler2D mapTexture;
layout (push_constant) uniform ubo {
	float fader;
};

layout (location=0) out vec4 FragColor;

layout (location=0) in vec2 texCoord;

void main(void)
{
	vec4 tex_color = texture(mapTexture,texCoord);
	tex_color.a *= fader;
	FragColor = tex_color;
}
