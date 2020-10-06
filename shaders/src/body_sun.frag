//
// body sun
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1) uniform sampler2D mapTexture;

layout(location=0) in vec2 TexCoord;

layout(location=0) out vec4 FragColor;
 
void main(void)
{
	vec3 tex_color = vec3(texture(mapTexture,TexCoord)).rgb;
	FragColor = vec4 (tex_color,1.0);
}
