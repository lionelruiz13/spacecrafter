//
//	pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec2 TexCoord;

layout (binding=0, set=1) uniform sampler2D texunit0;
layout (binding=1, set=1) uniform uboFrag {
	vec3 color;
};

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec4 tex_color = vec4(texture(texunit0,TexCoord)).rgba;
	tex_color.r *= color.r;
	tex_color.g *= color.g;
	tex_color.b *= color.b;
	FragColor = tex_color;
}

