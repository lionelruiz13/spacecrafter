//
//	pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)


in Interpolators
{
	vec2 TexCoord;
} interData;

layout (binding=0) uniform sampler2D texunit0;
uniform vec3 color;

out vec4 FragColor;

void main(void)
{
	vec4 tex_color = vec4(texture(texunit0,interData.TexCoord)).rgba;
	tex_color.r *= color.r;
	tex_color.g *= color.g;
	tex_color.b *= color.b;
	FragColor = tex_color;
}
