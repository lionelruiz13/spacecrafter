//vr360

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;
uniform float intensity;

vec3 tex_color;
vec3 yuv_color;
smooth in vec2 TexCoord;
 
out vec4 FragColor;
 
void main(void)
{
	vec3 tex_color = intensity * vec3(texture(texunit0,TexCoord)).rgb;
	//yuv_color = intensity * vec3(texture(texunit0,TexCoord)).rgb;
	//tex_color.r = yuv_color.r+1.13983*yuv_color.b;
	//tex_color.g = yuv_color.r-0.39465*yuv_color.g-0.58060*yuv_color.b;
	//tex_color.b = yuv_color.r+2.03211*yuv_color.g;
	FragColor = vec4 (tex_color,1.0);
}
