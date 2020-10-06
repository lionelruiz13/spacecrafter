//
//	VIDEO PLAYER
//
#version 420
#pragma debug(on)
#pragma optimize(off)

#include <convertToRGB.glsl>

layout (binding=0) uniform sampler2D s_tex_y;
layout (binding=1) uniform sampler2D s_tex_u;
layout (binding=2) uniform sampler2D s_tex_v;

layout (location=0) in vec2 TexCoord;

layout (binding=3) uniform ubo {
	vec4 noColor;
	float fader;
	bool transparency;
};

layout (location=0) out vec4 FragColor;

void main(void)
{
    vec3 tex_color = convertToRGB(s_tex_y, s_tex_u, s_tex_v, TexCoord);

	if (transparency) {
		vec3 diffVec = abs(tex_color-noColor.rgb);
		float delta = noColor.a;
		if ((diffVec.r<delta) && (diffVec.g<delta) && (diffVec.b<delta))
			discard;
	}

	FragColor = vec4(tex_color, fader);
}
