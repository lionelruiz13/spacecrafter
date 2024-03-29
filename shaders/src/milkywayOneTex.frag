//milkyway

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D texunit0;

layout (push_constant) uniform ubo {
	layout (offset=64) float cmag;
	float pollum;
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

void main(void)
{
	FragColor = vec4(max(vec3(texture(texunit0,TexCoord)) * cmag, vec3(pollum, 0.8 * pollum, 0.5 * pollum)), 1.f);
}
