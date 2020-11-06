//
//	nebulaHint
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;
layout (binding=1) uniform ubo {float fader;};

layout (location=0) in vec3 Color;
layout (location=1) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec4 tex_color = texture(mapTexture,TexCoord);
	//~ if (tex_color.r <0.2)
		//~ discard;

	FragColor = vec4(tex_color.r * Color.r, tex_color.g * Color.g, tex_color.b * Color.b, tex_color.a * fader);
}


