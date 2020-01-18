//
//	nebulaHint
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;
uniform float fader;
smooth in vec4 Color;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 tex_color = vec4 (texture(mapTexture,TexCoord));
	//~ if (tex_color.r <0.2)
		//~ discard;
	tex_color.r = tex_color.r * Color.r;
	tex_color.g = tex_color.g * Color.g;
	tex_color.b = tex_color.b * Color.b;
	tex_color.a *= fader;

	FragColor = tex_color;
}


