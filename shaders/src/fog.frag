//
// Fog
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0, set=1) uniform sampler2D texunit0;
layout (binding=2, set=1) uniform ubo {
	float sky_brightness;
	float fader;
};

vec4 tex_color;
layout (location=0) in vec2 TexCoord;

layout (location=0)out vec4 FragColor;
 
void main(void)
{
	vec4 tex_color = vec4(texture(texunit0,TexCoord)).rgba;
	if (tex_color.a == 0.0)
		discard;
	tex_color = tex_color * fader*(0.2f+0.1f*sky_brightness);
	FragColor = tex_color;
}
