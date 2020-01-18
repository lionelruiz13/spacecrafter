//vr360

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;
uniform float intensity;

vec3 tex_color;
smooth in vec2 TexCoord;
 
out vec4 FragColor;
 
void main(void)
{
	vec3 tex_color = intensity * vec3(texture(texunit0,TexCoord)).rgb;
	FragColor = vec4 (tex_color,1.0);
}
