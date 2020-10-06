// nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (push_constant) uniform ubo {
    layout (offset=64) float fader;
};
layout (binding=0) uniform sampler2D mapTexture;

layout (location=0) out vec4 FragColor;

layout (location=0) in vec2 texCoord;

void main(void)
{
	vec4 tex_color = vec4(texture(mapTexture,texCoord)).rgba;
	tex_color.a *= fader;
	//~ if (fader>1.)
		//~ FragColor = vec4(1.0, 0.0,0.0,1.0);
	//~ else
		FragColor = tex_color;
}
