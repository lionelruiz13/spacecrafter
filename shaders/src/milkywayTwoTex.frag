//milkyway

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D texunit0;
layout (binding=1, set=1) uniform sampler2D texunit1;

layout (push_constant) uniform ubo {
	layout (offset=64) float cmag;
	float texTransit;
};

layout (location=0) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;
 
void main(void)
{
	FragColor = vec4(mix(vec3(texture(texunit0,TexCoord)), vec3(texture(texunit1,TexCoord)), texTransit) * cmag, 1.f);
}
