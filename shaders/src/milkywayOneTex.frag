//milkyway

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D texunit0;
layout (binding=1, set=1) uniform sampler2D texunit1;

layout (binding=3, set=1) uniform ubo {
	float cmag;
	float texTransit;
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec3 tex_color0 = vec3(texture(texunit0,TexCoord)).rgb;
	FragColor = vec4(tex_color0 * cmag, 1.f);
}
